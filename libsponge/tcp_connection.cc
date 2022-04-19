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
    check_ackno_n_winsize_of_local_rcver();
    if(_receiver.ackno().has_value() 
    && seg.length_in_sequence_space() == 0 
    && hd.seqno == _receiver.ackno().value() - 1){
        _sender.send_empty_segment();
        check_ackno_n_winsize_of_local_rcver();
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
    check_ackno_n_winsize_of_local_rcver();
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
