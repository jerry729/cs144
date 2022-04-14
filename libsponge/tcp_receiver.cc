#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if(seg.header().syn){
        _isn = seg.header().seqno;
        if(_isn != WrappingInt32(UINT32_MAX)){
            _ack = _isn.value() + 1;
        }else{
            _ack = WrappingInt32(0);
        }
    }

    string pl = seg.payload().copy();
    if(_isn.has_value()){
        _reassembler.push_substring(pl, unwrap(seg.header().seqno, _isn.value(), _reassembler.expected_index()), seg.header().fin);
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    return _ack;
}

size_t TCPReceiver::window_size() const {
    return _capacity - _reassembler.stream_out().written_not_read();
}
