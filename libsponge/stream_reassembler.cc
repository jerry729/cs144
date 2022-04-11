#include "stream_reassembler.hh"

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

    size_t first_unacceptable = _expected_index + (_capacity - _output.written_not_read());

    string resData(data);
    size_t resIndex = index;

    if(index < _expected_index){
        if(index + data.size() - 1 < _expected_index){
            if(_eof){
                _output.end_input();
            }
            return;
        }
        //trunc overlapping data
        resData = resData.substr(_expected_index - index);
        resIndex = _expected_index;
    }
    if(resIndex + resData.size() > first_unacceptable){
        //trunc data, discard the bytes exceed the capacity
        resData = resData.substr(0, first_unacceptable - resIndex);
    }

    set<UnassembledSubstring>::iterator i;
    i = _Unassembled.lower_bound(UnassembledSubstring(resIndex, resData));
    while(i != _Unassembled.begin()){
        if(i == _Unassembled.end()){
            i--;
        }
        if(size_t dele_num = merge_substring(resIndex, resData, i)){
            _nUnassembled_bytes -= dele_num;
            if(i != _Unassembled.begin()){
                _Unassembled.erase(i--);
            }else{
                _Unassembled.erase(i);
                break;
            }
        }
        else{
            if(i != _Unassembled.begin()){
                i--;
            }else{
                break;
            }
        }
    }

    i = _Unassembled.lower_bound(UnassembledSubstring(resIndex, resData));
    while(i != _Unassembled.end()){
        if(size_t dele_num = merge_substring(resIndex, resData, i)){
            _nUnassembled_bytes -= dele_num;
            _Unassembled.erase(i++);
        }else{
            break;
        }
    }

    if(resIndex <= _expected_index){
        string final_data(resData.begin() - (_expected_index - resIndex), resData.end());
        size_t size_written2stream = _output.write(final_data);
        _expected_index += size_written2stream;
    }else{
        _Unassembled.insert(UnassembledSubstring(resIndex, resData));
        _nUnassembled_bytes += resData.size();
    }

    if(empty() && _eof){
        _output.end_input();
    }
    return;
}

int StreamReassembler::merge_substring(size_t& index, std::string& data, std::set<UnassembledSubstring>::iterator& it){
    size_t l2 = it -> index, r2 = l2 + (it -> substring).size() - 1;
    size_t l1 = index, r1 = l1 + data.size() - 1;

    if(l2 > r1 + 1 || l1 > r2 + 1){
        return 0;
    }

    size_t dele_num = (it -> substring).size();
    if(l2 > l1){
        if(r1 < r2){
        data += string((it->substring).begin() + r1 - l2, (it->substring).end());    
    }//else pass
    }else{
        string data2 = it -> substring;
        if(r1 > r2){
            data2 += string(data.begin() + r2 - l1, data.end());
        }
        data.assign(data2);
    }

    index = min(l2, l1);
    
    return dele_num;
}

size_t StreamReassembler::unassembled_bytes() const {
    return _nUnassembled_bytes;
}

bool StreamReassembler::empty() const {
    return unassembled_bytes() == 0;
}
