#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {

    const TCPHeader& header = seg.header();

    if(header.syn){
        _isn = header.seqno;
        if(_isn != WrappingInt32(UINT32_MAX)){
            _ack = _isn.value() + 1;
        }else{
            _ack = WrappingInt32(0);
        }
        auto& exptd_idx = _reassembler.expected_index();
        exptd_idx += 1;
    }

    if(!_isn.has_value()){
        return;
    }


    string pl = seg.payload().copy();
    _reassembler.push_substring(pl, unwrap(header.seqno, *_isn, _reassembler.last_assembled_index()), header.fin);
    _ack = wrap(_reassembler.expected_index(), _isn.value());

    if(header.fin){
        if(_ack != WrappingInt32(UINT32_MAX)){
            _ack = _ack.value() + 1;
        }else{
            _ack = WrappingInt32(0);
        }
    }

}

optional<WrappingInt32> TCPReceiver::ackno() const {
    return _ack;
}

size_t TCPReceiver::window_size() const {
    return _capacity - stream_out().written_not_read();
}
