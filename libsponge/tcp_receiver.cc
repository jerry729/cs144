#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    DUMMY_CODE(seg);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    return !_is_syn ? nullopt : optional<WrappingInt32>(_ack);
}

size_t TCPReceiver::window_size() const {
    return _capacity - _reassembler.stream_out().written_not_read();
}
