#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) 
    , _timer(retx_timeout){}

uint64_t TCPSender::bytes_in_flight() const {
    return _segments_outstanding.size();
}

void TCPSender::fill_window() {
    TCPSegment tcp_segment{};
    TCPHeader& header = tcp_segment.header();
    Buffer& payload = tcp_segment.payload();
    uint64_t payload_len = _window_size - bytes_in_flight();
    string data{};

    if( payload_len < 0 ){
        // there are not space vailable in the window
        return;
    }
    if(next_seqno() == _isn){
        // is syn byte in this seg?
        header.syn = true;
    }
    if(!_stream.buffer_empty()){
        //there still are new bytes to be read
        if(!_stream.input_ended()){
            if(_stream.buffer_size() >= payload_len){
                --payload_len;
            }else{
                payload_len = _stream.buffer_size();
            }
            payload_len = min(payload_len, TCPConfig::MAX_PAYLOAD_SIZE);
            data = _stream.read(payload_len);
        }else{
            if(_stream.buffer_size() + 2 <= payload_len){
                payload_len = _stream.buffer_size();
                if(payload_len <= TCPConfig::MAX_PAYLOAD_SIZE){
                    header.fin = true;
                    data = _stream.read(payload_len);
                }else{
                    data = _stream.read(TCPConfig::MAX_PAYLOAD_SIZE);
                }
            }else{
                payload_len = min(--payload_len, TCPConfig::MAX_PAYLOAD_SIZE);
                data = _stream.read(payload_len);
            }
        }
        payload = Buffer(move(data));
    }else{
        if(_stream.eof()){
            header.fin = true;
        }
    }

    _segments_out.push(tcp_segment);
    _segments_outstanding.push(tcp_segment);
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { DUMMY_CODE(ackno, window_size); }

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _timer.timer_tick(ms_since_last_tick);

    if(_timer.is_expired()){
        _segments_out.push(_segments_outstanding.front());
        if(_window_size > 0){
           _n_consecutive_retrans++;
           _timer.restart();
        }else{
            _timer.start();
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const {
    return _n_consecutive_retrans;
}

void TCPSender::send_empty_segment() {}
