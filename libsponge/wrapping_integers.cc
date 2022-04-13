#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint64_t u64_32max = 1ull << 32;
    uint32_t offset_from_isn = static_cast<uint32_t>(n % u64_32max);
    uint32_t isn_to_max = WrappingInt32(UINT32_MAX) - isn;
    WrappingInt32 res(0);
    if(offset_from_isn <= isn_to_max){
        res = isn + offset_from_isn;
    }else{
        res = WrappingInt32(offset_from_isn - isn_to_max - 1);
    }
    return res;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t tmp = 0;
    
    if(n.raw_value() < isn.raw_value()){
        tmp = static_cast<uint64_t>((n + 1ull + (WrappingInt32(UINT32_MAX) - isn)).raw_value());
    }else{
        tmp = static_cast<uint64_t>(n.raw_value() - isn.raw_value());
    }

    // while(tmp + (1ul << 32) < checkpoint){
    //     tmp += 1ul << 32;
    // }

    uint64_t s = checkpoint < tmp ? (tmp - checkpoint) : (checkpoint - tmp);
    uint64_t test = s / (1ul << 32) ;
    tmp += checkpoint < tmp ? test * (1ul << 32) : test * (1ul << 32);

    if(tmp < checkpoint){
        if(checkpoint - tmp > tmp + (1ul << 32) - checkpoint){
            tmp += 1ul << 32;
        }
    }else{
        if(tmp - checkpoint > tmp + (1ul << 32) - checkpoint){
            tmp += 1ul << 32;
        }
    }

    return tmp;
}
