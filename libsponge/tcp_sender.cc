#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) 
    , _window_size(0)
    , _first_not_acked(0)
    , _n_consecutive_retrans(0)
    , _eof_set(false)
    , _eof_sent(false)
    , _timer(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const {
    //printf("%lu \n", _segments_outstanding.back().length_in_sequence_space() + _segments_outstanding.back().header().seqno.raw_value() - _segments_outstanding.front().header().seqno.raw_value());
    if(_segments_outstanding.empty()){
        return 0;
    }
    size_t x = _segments_outstanding.back().length_in_sequence_space() + _segments_outstanding.back().header().seqno.raw_value() - _segments_outstanding.front().header().seqno.raw_value();
    return x;
}

void TCPSender::fill_window() {

    do{
        TCPSegment tcp_segment{};
        TCPHeader& header = tcp_segment.header();
        Buffer& payload = tcp_segment.payload();

        if( _window_size <= bytes_in_flight() && bytes_in_flight() != 0){
            // there are not space vailable in the window
            _eof_set = _eof_set | _stream.input_ended();
            return;
        }
        uint64_t payload_len = _window_size - bytes_in_flight();
        
        string data{};

        if(payload_len == 0){
            payload_len = 1;
        }

        if(next_seqno() == _isn){
            // is syn byte in this seg?
            header.syn = true;
            payload_len -= 1;
        }

        // string bs = _stream.peek_output(_stream.buffer_size());
        // printf("%s \n##################\n", bs.c_str());

        if(!_stream.buffer_empty()){
            //there still are new bytes to be read
            if(!(_eof_set | _stream.input_ended())){
                if(_stream.buffer_size() < payload_len){
                    payload_len = _stream.buffer_size();
                }
                payload_len = min(payload_len, TCPConfig::MAX_PAYLOAD_SIZE);
                data = _stream.read(payload_len);
            }else{
                if(_stream.buffer_size() <= payload_len - 1){
                    payload_len = _stream.buffer_size();
                    if(payload_len <= TCPConfig::MAX_PAYLOAD_SIZE){
                        header.fin = true;
                        _eof_sent = true;
                        data = _stream.read(payload_len);
                    }else{
                        data = _stream.read(TCPConfig::MAX_PAYLOAD_SIZE);
                        _eof_set = true;
                    }
                }else{
                    payload_len = min(payload_len, TCPConfig::MAX_PAYLOAD_SIZE);
                    data = _stream.read(payload_len);
                    _eof_set = true;
                }
            }
            payload = Buffer(move(data));
        }else{
            if((_eof_set | _stream.eof()) & (!_eof_sent)){
                header.fin = true;
                _eof_sent = true;
            }
        }

        if(tcp_segment.length_in_sequence_space() != 0){
            header.seqno = next_seqno();
            _next_seqno += tcp_segment.length_in_sequence_space();

            if(!_timer.is_started() && bytes_in_flight() == 0){
                _timer.start(_initial_retransmission_timeout);
            }
            _segments_out.push(tcp_segment);
            _segments_outstanding.push(tcp_segment);
            _first_not_acked = _segments_outstanding.front().header().seqno;
        }
    }while((next_seqno().raw_value() < (_first_not_acked + _window_size).raw_value()) && !_stream.buffer_empty());
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {

    _window_size = window_size;

    if(ackno.raw_value() <= _first_not_acked.raw_value()){
        return;
    }

    if(ackno.raw_value() > (_first_not_acked + bytes_in_flight()).raw_value()){
        //Impossible ackno (beyond next seqno) is ignored
        return;
    }

    // Delete segs that have been fully acked
    while(!_segments_outstanding.empty()){
        if((_first_not_acked + _segments_outstanding.front().length_in_sequence_space()).raw_value() <= ackno.raw_value()){
            _first_not_acked = _first_not_acked + _segments_outstanding.front().length_in_sequence_space();
            _segments_outstanding.pop();
        }else{
            break;
        }
    }
 
    _n_consecutive_retrans = 0;
    if(bytes_in_flight() > 0){
        _timer.start(_initial_retransmission_timeout);
    }else{
        _timer.stop();
    }

    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _timer.timer_tick(ms_since_last_tick);

    if(_timer.is_expired()){
        if(!_segments_outstanding.empty()){
            _segments_out.push(_segments_outstanding.front());
            if((!(_window_size == 0)) || _segments_outstanding.front().header().syn){
                _n_consecutive_retrans++;
                _timer.restart();
            }else{
                _timer.start();
            }  
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const {
    return _n_consecutive_retrans;
}

void TCPSender::send_empty_segment() {
    TCPSegment seg{};
    seg.header().seqno = next_seqno();

    _segments_out.push(seg);
}

bool TCPSender::eof_sent() const{
    return _eof_sent;
}
