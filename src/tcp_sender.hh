#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <map>
#include <utility>

class TCPTimer
{
  uint64_t time_now_ms_ = 0;
  uint64_t time_expired_ms_ = 0;

  uint64_t initial_RTO_ms_;
  uint64_t RTO_ms_;

  bool zero_size_window_ = false;
  bool running_ = false;

public:
  TCPTimer( uint64_t initial_RTO_ms );

  void reset_RTO();

  void double_RTO();

  void update_time( uint64_t elapsed_time_ms );

  bool is_expired() const;

  void start();

  void stop();

  void set_zero_size_window(bool zw);
};

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  std::map<uint64_t, TCPSenderMessage> wait_sending_q_ = {};
  std::map<uint64_t, TCPSenderMessage> wait_ack_q_ = {};
  uint64_t nseq_in_flight_ = 0;
  uint64_t nconsecutive_retransmisson_ = 0;

  uint64_t window_start_ = 0;
  uint64_t window_end_ = 1;
  uint16_t window_size_ = 1;
  uint64_t seq_checkpoint_ = 0;

  TCPTimer timer_;

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?

private:
  /* Push message into wait_sending_q_ */
  void push_message( bool SYN, std::string&& payload, bool FIN );
};
