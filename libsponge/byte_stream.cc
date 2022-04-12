#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : stream_()
    , send_base_{0}
    , next_seq_num_{0}
    , window_size_{capacity}
    , eof_{false} {}

size_t ByteStream::write(const string &data) {
    size_t l = min(data.size(), remaining_capacity());
    for (size_t i = 0; i < l; i++){
        stream_.emplace_back(data[i]);
    }
    next_seq_num_ += l;
    return l;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string ret;
    size_t l = min(len, written_not_read());
    for(size_t i = 0; i < l; i++){
        ret.push_back(stream_[i]);
    }
    return ret;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t l = min(len, written_not_read());
    for(size_t i = 0; i < l; ++i){
        stream_.pop_front();
    }

    send_base_ += l;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string ret;
    size_t l = min(len, written_not_read());
    for(size_t i = 0; i < l; i++){
        ret.push_back(stream_.front());
        stream_.pop_front();
    }

    send_base_ += l;
    return ret;
}

void ByteStream::end_input() {
    eof_ = true;
}

bool ByteStream::input_ended() const {
    return eof_;
}

size_t ByteStream::buffer_size() const {
    return written_not_read();
}

bool ByteStream::buffer_empty() const {
    return written_not_read() == 0;
}

bool ByteStream::eof() const {
    return input_ended() && buffer_empty();
}

size_t ByteStream::bytes_written() const {
    return next_seq_num_;
}

size_t ByteStream::bytes_read() const {
    return send_base_;
}

size_t ByteStream::remaining_capacity() const { 
    return window_size_ - written_not_read();
}

size_t ByteStream::written_not_read() const{
    return next_seq_num_ - send_base_;
}
