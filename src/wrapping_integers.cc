#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return Wrap32 { static_cast<uint32_t>( n & 0xFFFFFFFF ) + zero_point.raw_value_ };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint32_t checkpoint_seqno = ( checkpoint & 0xFFFFFFFF ) + zero_point.raw_value_;
  uint32_t d_left
    = checkpoint_seqno >= raw_value_ ? checkpoint_seqno - raw_value_ : 0x100000000 - raw_value_ + checkpoint_seqno;
  uint32_t d_right = 0x100000000 - d_left;

  if ( d_left < d_right ) {
    return checkpoint + ( checkpoint < d_left ? 0x100000000 : 0 ) - d_left;
  } else {
    return checkpoint + d_right;
  }
}
