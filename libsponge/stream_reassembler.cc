#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) 
: _output(capacity)
, _capacity(capacity)
, _expected_index(0)
, _nUnassembled_bytes(0)
, _eof(false) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    _eof = eof;
    if(data.size() > _capacity){
        _eof = false;
    }
    //还是忽略了一个问题，set内确实不会有重复和乱序，但是如果已经写入了bytestream又来了一个重复的
    if(empty()){
        if (index == _expected_index && data.size() <= _output.remaining_capacity()){
            _output.write(data);
        }
        else if(data.size() + unassembled_bytes() < remaining_unassembled_capacity()){

        }
        else if(index > expected_index() && data.size() <= _capacity){
            _auxiliary_storage.insert(UnassembledSubstring(index, data));
        }
        //else discard
    }
    else{
        if(data.size() <= _capacity){
            _auxiliary_storage.insert(UnassembledSubstring(index, data));
        }
        // else discard
        if(_auxiliary_storage.find(UnassembledSubstring(expected_index())) != _auxiliary_storage.end()){
            //find max_ordered and max_ordered_byte_size
            set<UnassembledSubstring>::iterator it;
            size_t max_ordered_byte_size = 0;
            for(it = _auxiliary_storage.begin(); it != _auxiliary_storage.end(); it++){
                if((*it).index + (*it).substring.size() + 1 != (*(it++)).index){
                    break;
                }
                if(max_ordered_byte_size + (*it).substring.size() > _output.remaining_capacity()){
                    it--;
                    break;
                }else{
                    max_ordered_byte_size += (*it).substring.size();
                }
            }
            //write a max_oredered_size number of bytes into ByteStream
            set<UnassembledSubstring>::iterator it2;
            for(it2 = _auxiliary_storage.begin(); it2 != it; it2++){
                _output.write((*it2).substring);
                _auxiliary_storage.erase(it2);
            }
        }
        //else stay in auxiliary storage
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    return _nUnassembled_bytes;
}

bool StreamReassembler::empty() const {
    return unassembled_bytes() == 0;
}
