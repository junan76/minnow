#include "tcp_receiver.hh"

#include <assert.h>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.

  /* 记录SYN标识 */
  if ( message.SYN ) {
    initial_seq_ = message.seqno;
    syn_received_ = true;
  }

  // 目前为止还没有收到SYN, 则忽略当前包
  if ( syn_received_ == false ) {
    return;
  }

  /* 接收当前包的数据 */

  Wrap32 seqno = message.seqno;
  if ( message.SYN ) {
    seqno = seqno + 1;
  }
  string data = message.payload;
  bool is_last_substring = message.FIN;

  Reader& reader = inbound_stream.reader();
  uint64_t checkpoint = reader.bytes_popped() + reader.bytes_buffered() + 1;
  uint64_t absolute_seqno = seqno.unwrap( initial_seq_, checkpoint );

  // 忽略序列号错误的包
  if ( absolute_seqno == 0 ) {
    return;
  }
  reassembler.insert( absolute_seqno - 1, data, is_last_substring, inbound_stream );

  /* 更新ackno */
  checkpoint = reader.bytes_popped() + reader.bytes_buffered() + 1;
  checkpoint += inbound_stream.is_closed();
  ack_seq_ = Wrap32::wrap( checkpoint, initial_seq_ );
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  TCPReceiverMessage result;
  if ( syn_received_ ) {
    result.ackno = ack_seq_;
  }

  uint64_t capacity = inbound_stream.available_capacity();
  result.window_size = capacity <= UINT16_MAX ? capacity : UINT16_MAX;
  return result;
}
