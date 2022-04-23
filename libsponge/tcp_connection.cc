#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const {
    return _sender.stream_in().remaining_capacity();
}

size_t TCPConnection::bytes_in_flight() const {
    return _sender.bytes_in_flight();
}

size_t TCPConnection::unassembled_bytes() const {
    return _receiver.unassembled_bytes();
}

size_t TCPConnection::time_since_last_segment_received() const {
    return _ms_since_last_seg_rcved;
}

void TCPConnection::segment_received(const TCPSegment &seg) {
    _ms_since_last_seg_rcved = 0;
    const TCPHeader& hd = seg.header();
    if(hd.rst){
        set_tcp_connection_error();
        return;
    }
    _receiver.segment_received(seg);
    _sender.ack_received(hd.ackno, hd.win);
    check_ackno_n_winsize_of_local_rcver();

    // If the incoming segment occupied any sequence numbers, the TCPConnection makes sure that at least one segment is sent in reply, to reflect an update in the ackno and window size.
    if(seg.header().syn
    && _sender.syn_sent()){
        send_ack_for_syn();
        _is_active = true;
        return;
    }

    if(seg.header().syn
    && (!_sender.syn_sent())){
        _sender.fill_window();
        _is_active = true;
        _sender.syn_sent() = true;
        check_ackno_n_winsize_of_local_rcver();
        return;
    }

    // bad ack - early seg out
    if(_receiver.ackno().has_value()
    && _receiver.bad_ack()
    && hd.seqno == _receiver.ackno().value() - 1){
        _sender.send_empty_segment();
        check_ackno_n_winsize_of_local_rcver();
        _receiver.bad_ack() = false;
        return;
    }

    // bad ack - late seg out
    if(_receiver.ackno().has_value()
    && hd.seqno.raw_value() >= (_receiver.ackno().value() + _receiver.window_size()).raw_value()
    && (!(_receiver.window_size() == 0))){
        printf("%u %u \n",  hd.seqno.raw_value(), (_receiver.ackno().value() + _receiver.window_size()).raw_value());
        _sender.send_empty_segment();
        check_ackno_n_winsize_of_local_rcver();
        return;
    }else if(_receiver.ackno().has_value()
    && hd.seqno.raw_value() > _receiver.ackno().value().raw_value()
    && hd.seqno.raw_value() < (_receiver.ackno().value() + _receiver.window_size()).raw_value()
    && (!hd.fin)){
        _sender.send_empty_segment();
        check_ackno_n_winsize_of_local_rcver();
        return;
    }

    // bad ack
    if(_receiver.ackno().has_value()
    && hd.seqno.raw_value() == (_receiver.ackno().value() - 1).raw_value()
    && (!hd.fin)){
        _sender.send_empty_segment();
        check_ackno_n_winsize_of_local_rcver();
        return;
    }

    //linger end
    if(seg.header().fin 
    && _sender.eof_sent() 
    && _receiver.ackno().has_value()
    && hd.seqno == _receiver.ackno().value() - 1){
        _sender.send_empty_segment();
        check_ackno_n_winsize_of_local_rcver();
        return;
    }

    //passive close
    if(seg.header().fin 
    && (!_sender.eof_sent()) 
    && (!_sender.stream_in().eof())
    && _receiver.ackno().has_value()
    && hd.seqno == _receiver.ackno().value() - 1){
        _linger_after_streams_finish = false;
        _sender.send_empty_segment();
        check_ackno_n_winsize_of_local_rcver();
        return;
    }

    //segment out of the window
    
    // Keep-alive seg  
    if(_receiver.ackno().has_value() 
    && seg.length_in_sequence_space() == 0 
    && hd.seqno == _receiver.ackno().value() - 1){
        _sender.send_empty_segment();
        check_ackno_n_winsize_of_local_rcver();
        return;
    }

    if(_receiver.ackno().has_value()
    && seg.length_in_sequence_space() != 0
    && (!hd.syn) && (!hd.fin)
    && (hd.seqno + seg.length_in_sequence_space()).raw_value() == _receiver.ackno().value().raw_value()){
        _sender.send_empty_segment();
        check_ackno_n_winsize_of_local_rcver();
        return;
    }

    if(_receiver.ackno().has_value()
    && seg.length_in_sequence_space() != 0
    && (!hd.syn) && (!hd.fin)
    && _receiver.ack_updated()){
        _receiver.ack_updated() = false;
        _sender.send_empty_segment();
        check_ackno_n_winsize_of_local_rcver();
        return;
    }
}

bool TCPConnection::active() const {
    return _is_active;
}

size_t TCPConnection::write(const string &data) {
    size_t w_size = _sender.stream_in().write(data);
    _sender.fill_window();
    check_ackno_n_winsize_of_local_rcver();
    return w_size;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _sender.tick(ms_since_last_tick);
    _ms_since_last_seg_rcved += ms_since_last_tick;
    
    if(_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS){
        send_empty_seg_with_rst(true);
        set_tcp_connection_error();
        return;
    }
    
    if((!_sender.eof_sent()) && _receiver.stream_out().eof()){
        _linger_after_streams_finish = false;
    }

    if(is_clean_shutdown_prereq1_satisfied()
    && is_clean_shutdown_prereq2_satisfied()
    && is_clean_shutdown_prereq3_satisfied()){
        if(!_linger_after_streams_finish){
            _is_active = false;
        }else{
            if(time_since_last_segment_received() >= 10 * _cfg.rt_timeout){
                _is_active = false;
            }
        }
    }

    check_ackno_n_winsize_of_local_rcver();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    check_ackno_n_winsize_of_local_rcver();
}

void TCPConnection::connect() {
    _is_active = true;
    _sender.fill_window();
    check_ackno_n_winsize_of_local_rcver();
}

void TCPConnection::check_ackno_n_winsize_of_local_rcver(){
    while(!_sender.segments_out().empty()){
        TCPSegment seg = _sender.segments_out().front();
        TCPHeader& hd = seg.header();
        if(_receiver.ackno().has_value()){
            hd.ack = true;
            hd.ackno = _receiver.ackno().value();
        }
        hd.win = _receiver.window_size() <= std::numeric_limits<uint16_t>::max() ? _receiver.window_size() : std::numeric_limits<uint16_t>::max();
        _sender.segments_out().pop();
        _segments_out.push(seg);
    }
}

void TCPConnection::set_tcp_connection_error(){
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _linger_after_streams_finish = false;
    _is_active = false;
}

bool TCPConnection::is_clean_shutdown_prereq1_satisfied() const {
    return (_receiver.unassembled_bytes() == 0 && _receiver.stream_out().eof());
}

bool TCPConnection::is_clean_shutdown_prereq2_satisfied() const {
    return (_sender.stream_in().eof() && _sender.eof_sent());
}

bool TCPConnection::is_clean_shutdown_prereq3_satisfied() const {
    return (_sender.bytes_in_flight() == 0);
}

void TCPConnection::send_empty_seg_with_rst(bool reset) {
    if(reset == true){
        TCPSegment seg{};
        seg.header().rst = true;
        seg.header().seqno = _sender.next_seqno();
        _segments_out.push(seg);
    }
}

void TCPConnection::send_ack_for_syn(){
    TCPSegment seg{};
    seg.header().seqno = _sender.next_seqno();
    seg.header().ack = true;
    seg.header().ackno = _receiver.ackno().value();
    seg.header().win = _receiver.window_size();
    _segments_out.push(seg);
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            send_empty_seg_with_rst(true);
            set_tcp_connection_error();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
