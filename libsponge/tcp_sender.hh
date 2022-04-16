#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>

//! \brief The "sender" part of a TCP implementation.

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
class TCPSender {
  private:
    //! our initial sequence number, the number for our SYN.
    WrappingInt32 _isn;

    //! outbound queue of segments that the TCPSender wants sent
    std::queue<TCPSegment> _segments_out{};
    std::queue<TCPSegment> _segments_outstanding{};

    //! retransmission timer for the connection
    unsigned int _initial_retransmission_timeout;

    //! outgoing stream of bytes that have not yet been sent
    ByteStream _stream;

    //! the (absolute) sequence number for the next byte to be sent
    uint64_t _next_seqno{0};

    uint64_t _window_size;
    WrappingInt32 _first_not_acked;
    size_t _n_consecutive_retrans;

    RetransmissionTimer _timer;

  public:
    //! Initialize a TCPSender
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});

    //! \name "Input" interface for the writer
    //!@{
    ByteStream &stream_in() { return _stream; }
    const ByteStream &stream_in() const { return _stream; }
    //!@}

    //! \name Methods that can cause the TCPSender to send a segment
    //!@{

    //! \brief A new acknowledgment was received
    void ack_received(const WrappingInt32 ackno, const uint16_t window_size);

    //! \brief Generate an empty-payload segment (useful for creating empty ACK segments)
    void send_empty_segment();

    //! \brief create and send segments to fill as much of the window as possible
    void fill_window();

    //! \brief Notifies the TCPSender of the passage of time
    void tick(const size_t ms_since_last_tick);
    //!@}

    //! \name Accessors
    //!@{

    //! \brief How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //! \note count is in "sequence space," i.e. SYN and FIN each count for one byte
    //! (see TCPSegment::length_in_sequence_space())
    size_t bytes_in_flight() const;

    //! \brief Number of consecutive retransmissions that have occurred in a row
    unsigned int consecutive_retransmissions() const;

    //! \brief TCPSegments that the TCPSender has enqueued for transmission.
    //! \note These must be dequeued and sent by the TCPConnection,
    //! which will need to fill in the fields that are set by the TCPReceiver
    //! (ackno and window size) before sending.
    std::queue<TCPSegment> &segments_out() { return _segments_out; }
    //!@}

    //! \name What is the next sequence number? (used for testing)
    //!@{

    //! \brief absolute seqno for the next byte to be sent
    uint64_t next_seqno_absolute() const { return _next_seqno; }

    //! \brief relative seqno for the next byte to be sent
    WrappingInt32 next_seqno() const { return wrap(_next_seqno, _isn); }
    //!@}
};

//! \brief The timer maitained by TCPSender

//! The only job is to maintain the TCP_sender_total_alive_time and the duration since timer started ( start method called ).
//! It can automatically terminate when timeout
class RetransmissionTimer{
  private:
    unsigned int _initial_rto;
    unsigned int _rto;

    bool _is_started;
    bool _is_timeout;

    size_t _tcpsender_total_alive_time;
    size_t _start_tmstmp;

  public:

    RetransmissionTimer(unsigned int initial_rto)
    : _initial_rto(initial_rto)
    , _rto(initial_rto)
    , _is_started(false)
    , _is_timeout(false) 
    , _tcpsender_total_alive_time(0) 
    , _start_tmstmp(0) {}

    //! This overloading is for Round Trip Time re-evaluation
    void start(const unsigned int& initial_rto){
      _is_started = true;
      _is_timeout = false;
      reset_init_inverval(initial_rto);
      _start_tmstmp = _tcpsender_total_alive_time;
    }

    //! Timer starts when accepting data from upper layer or when accepting ACK
    void start(){
      _is_started = true;
      _is_timeout = false;
      _rto = _initial_rto;
      _start_tmstmp = _tcpsender_total_alive_time;
    }

    void stop(){
      _is_started = false;
      _start_tmstmp = 0;
    }

    void timer_tick(const size_t& ms_since_last_tick) {
      _tcpsender_total_alive_time += ms_since_last_tick;
      if(_tcpsender_total_alive_time - _start_tmstmp > _rto){
        _is_timeout = true;
        _is_started = false;
      }
    }

    const bool& is_expired(){
      return _is_timeout;
    }

    void reset_double_interval(){
      _rto *= 2;
    }

    void reset_init_inverval(const unsigned int& initial_rto){
      _initial_rto = initial_rto;
      _rto = _initial_rto;
    }

    //! The difference between restart and start is that restart would keep the doubled timeout interval and it was expected to call when retransmit an unacked seg
    void restart(){
      _is_timeout = false;
      _is_started = true;
      reset_double_interval();
      _start_tmstmp = _tcpsender_total_alive_time;
    }

    bool is_started(){
      return _is_started;
    }

};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
