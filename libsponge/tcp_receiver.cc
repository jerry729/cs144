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
        _syn_rcved = true;
        _isn = header.seqno;
        if(_isn != WrappingInt32(UINT32_MAX)){
            _ack = _isn.value() + 1;
        }else{
            _ack = WrappingInt32(0);
        }
    }
    _fin_rcved = _fin_rcved | (header.fin & _syn_rcved);

    //int tst_i = 0;
    //printf("--%d-- %u %u\n",tst_i, _isn -> raw_value(), header.seqno.raw_value());

    if(!_isn.has_value()){
        return;
    }


    string pl = seg.payload().copy();
    uint64_t tmp = unwrap(header.seqno, *_isn, _reassembler.expected_index());
    tmp = header.syn ? tmp : tmp - 1;
    //printf("++%d++ tmp:%lu isn:%u seq:%u\n",++tst_i, tmp, _isn -> raw_value(), header.seqno.raw_value());
    
    //printf("++%d++ ack:%u\n",++tst_i, _ack->raw_value());

    _reassembler.push_substring(pl, tmp, header.fin);
    _ack = wrap(_reassembler.expected_index() + 1, _isn.value());
    //printf("--%d-- ack:%u\n",++tst_i, _ack->raw_value());

    
    if(_fin_rcved && _reassembler.empty()){
        if(_ack != WrappingInt32(UINT32_MAX)){
            _ack = _ack.value() + 1;
        }else{
            _ack = WrappingInt32(0);
        }
    }

    //printf("####################################\n");

}

optional<WrappingInt32> TCPReceiver::ackno() const {
    return _ack;
}

size_t TCPReceiver::window_size() const {
    return _capacity - stream_out().written_not_read();
}
