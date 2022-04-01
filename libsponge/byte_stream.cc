#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity):stream_(capacity), send_base_{0}, next_seq_num_{0}, window_size_{capacity}, rcv_base_{0}, eof_{false}, written_n_{0}, read_n_{0}, buffer_size_{0} {}

size_t ByteStream::write(const string &data) {
    size_t l = min(data.size(), remaining_capacity());
    for (size_t i = 0; i < l; i++){
        stream_.emplace_back(data[i]);
    }
    next_seq_num_ += l;
    written_n_ += l;
    return l;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    DUMMY_CODE(len);
    return {};
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { DUMMY_CODE(len); }

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    DUMMY_CODE(len);
    return {};
}

void ByteStream::end_input() {
    eof_ = true;
}

bool ByteStream::input_ended() const {
    return eof_;
}

size_t ByteStream::buffer_size() const {
    return buffer_size_;
}

bool ByteStream::buffer_empty() const {
    return buffer_size_ == 0;
}

bool ByteStream::eof() const {
    return input_ended() && buffer_empty();
}

size_t ByteStream::bytes_written() const {
    return written_n_;
}

size_t ByteStream::bytes_read() const {
    return read_n_;
}

size_t ByteStream::remaining_capacity() const { 
    return window_size_ - next_seq_num_;
}
