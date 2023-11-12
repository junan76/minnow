#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity )
{
  buf_.resize( capacity_ + 1 );
}

void Writer::push( string data )
{
  // Your code here.
  uint64_t nwrite = min( data.size(), available_capacity() );

  for ( uint64_t i = 0; i < nwrite; i++ ) {
    buf_[write_pos_] = data[i];
    write_pos_ += 1;
    write_pos_ %= ( capacity_ + 1 );
  }

  total_pushed_ += nwrite;
}

void Writer::close()
{
  // Your code here.
  closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  if ( read_pos_ <= write_pos_ ) {
    return capacity_ + read_pos_ - write_pos_;
  } else {
    return read_pos_ - write_pos_ - 1;
  }
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return total_pushed_;
}

string_view Reader::peek() const
{
  // Your code here.
  if ( read_pos_ <= write_pos_ ) {
    return string_view( buf_.data() + read_pos_, write_pos_ - read_pos_ );
  } else {
    return string_view( buf_.data() + read_pos_, capacity_ - read_pos_ + 1 );
  }
}

bool Reader::is_finished() const
{
  // Your code here.
  return closed_ and bytes_buffered() == 0;
}

bool Reader::has_error() const
{
  // Your code here.
  return error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  uint64_t nread = min( len, bytes_buffered() );

  if ( read_pos_ <= write_pos_ ) {
    read_pos_ += nread;
  } else {
    if ( nread <= capacity_ - read_pos_ + 1 ) {
      read_pos_ += nread;
      read_pos_ %= ( capacity_ + 1 );
    } else {
      read_pos_ = nread - ( capacity_ - read_pos_ + 1 );
    }
  }

  total_poped_ += nread;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  if ( read_pos_ <= write_pos_ ) {
    return write_pos_ - read_pos_;
  } else {
    return capacity_ + write_pos_ + 1 - read_pos_;
  }
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return total_poped_;
}
