#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  // Your code here.
  uint64_t n_write = min( data.size(), capacity_ - buf_.size() );
  buf_.insert( buf_.end(), data.begin(), data.begin() + n_write );
  total_pushed_ += n_write;
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
  return capacity_ - buf_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return total_pushed_;
}

string_view Reader::peek() const
{
  // Your code here.
  // TODO: how to use string_view, deque ???
  return string_view(&buf_.front(), 1);
}

bool Reader::is_finished() const
{
  // Your code here.
  return closed_ and buf_.size() == 0;
}

bool Reader::has_error() const
{
  // Your code here.
  return error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  uint64_t n_read = min( len, buf_.size() );
  buf_.erase( buf_.cbegin(), buf_.cbegin() + n_read );
  total_poped_ += n_read;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buf_.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return total_poped_;
}
