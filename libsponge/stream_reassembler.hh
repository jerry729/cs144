#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH

#include "byte_stream.hh"

#include <cstdint>
#include <string>
#include <set>

class UnassembledSubstring{
  public:
  size_t _index;
  std::string _substring;

  UnassembledSubstring(const size_t index, const std::string& substring)
  : _index(index)
  , _substring(substring) {}

  bool operator<(const UnassembledSubstring& other ) const{
    return this -> _index < other._index;
  }
};

//! \brief A class that assembles a series of excerpts from a byte stream (possibly out of order,
//! possibly overlapping) into an in-order byte stream.
class StreamReassembler {
  private:

    ByteStream _output;  //!< The reassembled in-order byte stream
    size_t _capacity;    //!< The maximum number of bytes
    size_t _expected_index;
    size_t _nUnassembled_bytes;
    bool _eof;
    std::set<UnassembledSubstring> _Unassembled;

  public:
    //! \brief Construct a `StreamReassembler` that will store up to `capacity` bytes.
    //! \note This capacity limits both the bytes that have been reassembled,
    //! and those that have not yet been reassembled.
    StreamReassembler(const size_t capacity);

    //! \brief Receive a substring and write any newly contiguous bytes into the stream.
    //!
    //! The StreamReassembler will stay within the memory limits of the `capacity`.
    //! Bytes that would exceed the capacity are silently discarded.
    //!
    //! \param data the substring
    //! \param index indicates the index (place in sequence) of the first byte in `data`
    //! \param eof the last byte of `data` will be the last byte in the entire stream
    void push_substring(const std::string &data, const size_t index, const bool eof);
    int merge_substring(size_t& index, std::string& data, std::set<UnassembledSubstring>::iterator& it);

    //! \name Access the reassembled byte stream
    //!@{
    const ByteStream &stream_out() const { return _output; }
    ByteStream &stream_out() { return _output; }
    //!@}

    //! The number of bytes in the substrings stored but not yet reassembled
    //!
    //! \note If the byte at a particular index has been pushed more than once, it
    //! should only be counted once for the purpose of this function.
    size_t unassembled_bytes() const;

    //! \brief Is the internal state empty (other than the output stream)?
    //! \returns `true` if no substrings are waiting to be assembled
    bool empty() const;

    size_t last_assembled_index() const { return _expected_index == 0 ? 0 : _expected_index - 1; }

    size_t& expected_index() { return _expected_index; }

    bool is_eof() const { return _eof; }
};

#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
