#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {

    const TCPHeader& header = seg.header();
    uint64_t old_ack = _reassembler.expected_index();

    if(header.syn){
        _syn_rcved = true;
        _isn = header.seqno;
        if(_isn != WrappingInt32(UINT32_MAX)){
            _ack = _isn.value() + 1;
        }else{
            _ack = WrappingInt32(0);
        }
    }
    _fin_rcved = _fin_rcved | (header.fin & _syn_rcved);


    if(!_isn.has_value()){
        return;
    }


    string pl = seg.payload().copy();
    uint64_t tmp = unwrap(header.seqno, *_isn, _reassembler.expected_index());

    if(tmp == 0 && (!header.syn)){
        _bad_ack = true;
        return;
    }

    tmp = (header.syn || tmp == 0) ? tmp : tmp - 1;

    _reassembler.push_substring(pl, tmp, header.fin);

    if(old_ack != _reassembler.expected_index()){
        _ack_updated = true;
    }
    
    _ack = wrap(_reassembler.expected_index() + 1, _isn.value());

    if(_fin_rcved && _reassembler.empty()){
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

bool& TCPReceiver::ack_updated(){
    return _ack_updated;
}
