#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <assert.h>
#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) )
  , initial_RTO_ms_( initial_RTO_ms )
  , timer_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return nseq_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return nconsecutive_retransmisson_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  auto itr = wait_sending_q_.begin();
  if ( itr != wait_sending_q_.end() ) {
    TCPSenderMessage result = itr->second;

    wait_ack_q_.insert( { itr->first, result } );
    wait_sending_q_.erase( itr++ );
    timer_.start();

    return result;
  }

  return {};
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  static bool FIN_flag = false;
  if ( seq_checkpoint_ == 0 ) {
    FIN_flag = outbound_stream.is_finished() || outbound_stream.has_error();
    push_message( true, {}, FIN_flag );
    if ( FIN_flag ) {
      return;
    }
  }

  if ( ( outbound_stream.is_finished() || outbound_stream.has_error() ) && window_end_ > seq_checkpoint_
       && FIN_flag == false ) {
    FIN_flag = true;
    push_message( false, {}, FIN_flag );
    return;
  }

  while ( window_end_ > seq_checkpoint_ ) {
    std::string s { outbound_stream.peek() };
    size_t n = min( s.length(), window_end_ - seq_checkpoint_ );
    n = min( n, TCPConfig::MAX_PAYLOAD_SIZE );

    if ( n == 0 ) {
      break;
    } else {
      outbound_stream.pop( n );
      FIN_flag = ( outbound_stream.is_finished() || outbound_stream.has_error() )
                 && n + 1 <= window_end_ - seq_checkpoint_;
      push_message( false, s.substr( 0, n ), FIN_flag );
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  Wrap32 seqno = Wrap32::wrap( seq_checkpoint_, isn_ );
  return { seqno, false, {}, false };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  if ( msg.ackno.has_value() ) {
    window_start_ = msg.ackno.value().unwrap( isn_, seq_checkpoint_ );
    if ( window_start_ > seq_checkpoint_ ) {
      return;
    }
  }
  timer_.set_zero_size_window( msg.window_size == 0 );
  window_size_ = msg.window_size == 0 ? 1 : msg.window_size;
  window_end_ = window_start_ + window_size_;

  /* 删除已经收到ack的包 */
  bool effective_ack = false;
  for ( auto itr = wait_ack_q_.begin(); itr != wait_ack_q_.end(); ) {
    uint64_t last = itr->first + itr->second.sequence_length();
    if ( last <= window_start_ ) {
      nseq_in_flight_ -= itr->second.sequence_length();
      wait_ack_q_.erase( itr++ );
      effective_ack = true;
    } else {
      break;
    }
  }

  if ( effective_ack ) {
    nconsecutive_retransmisson_ = 0;
    timer_.reset_RTO();
  }

  if ( wait_ack_q_.empty() ) {
    timer_.stop();
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  timer_.update_time( ms_since_last_tick );

  if ( timer_.is_expired() && !wait_ack_q_.empty() ) {
    auto itr = wait_ack_q_.begin();

    wait_sending_q_.insert( { itr->first, itr->second } );
    wait_ack_q_.erase( itr );
    timer_.double_RTO();

    nconsecutive_retransmisson_++;
  }
}

void TCPSender::push_message( bool SYN, std::string&& payload, bool FIN )
{
  TCPSenderMessage msg;

  msg.seqno = Wrap32::wrap( seq_checkpoint_, isn_ );
  msg.SYN = SYN;
  msg.payload = payload;
  msg.FIN = FIN;
  uint64_t n = msg.sequence_length();

  wait_sending_q_.insert( { seq_checkpoint_, msg } );

  nseq_in_flight_ += n;
  seq_checkpoint_ += n;
}

TCPTimer::TCPTimer( uint64_t initial_RTO_ms ) : initial_RTO_ms_( initial_RTO_ms ), RTO_ms_( initial_RTO_ms ) {}

void TCPTimer::reset_RTO()
{
  RTO_ms_ = initial_RTO_ms_;
  if ( running_ ) {
    stop();
    start();
  }
}

void TCPTimer::double_RTO()
{
  if ( zero_size_window_ == false ) {
    RTO_ms_ *= 2;
  }

  if ( running_ ) {
    stop();
    start();
  }
}

void TCPTimer::update_time( uint64_t elapsed_time_ms )
{
  time_now_ms_ += elapsed_time_ms;
}

bool TCPTimer::is_expired() const
{
  if ( running_ == false ) {
    return false;
  }
  return time_now_ms_ >= time_expired_ms_;
}

void TCPTimer::start()
{
  if ( running_ ) {
    return;
  }

  running_ = true;
  time_expired_ms_ = time_now_ms_ + RTO_ms_;
}

void TCPTimer::stop()
{
  running_ = false;
  time_expired_ms_ = 0;
}

void TCPTimer::set_zero_size_window( bool zw )
{
  zero_size_window_ = zw;
}