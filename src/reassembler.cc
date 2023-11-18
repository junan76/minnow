#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  uint64_t first_unpoped = output.reader().bytes_popped();
  uint64_t first_unassembled = first_unpoped + output.reader().bytes_buffered();
  uint64_t first_unacceptable = first_unassembled + output.available_capacity();

  // 截取字符串
  uint64_t last_index = first_index + data.size();
  if ( last_index < first_unassembled || ( first_index < last_index && last_index == first_unassembled )
       || first_index >= first_unacceptable ) {
    return;
  }

  uint64_t start = max( first_index, first_unassembled );
  uint64_t end = min( last_index, first_unacceptable );
  data = data.substr( start - first_index, end - start );

  // 首先插入到内部存储之中
  if ( internal_map_.empty() ) {
    internal_map_.insert( { start, std::make_pair( data, is_last_substring ) } );
  } else {
    bool inserted = false;
    bool erased = false;
    for ( auto itr = internal_map_.begin(); itr != internal_map_.end(); ) {
      uint64_t start_itr = itr->first;
      uint64_t end_itr = start_itr + itr->second.first.size();
      std::string data_itr = itr->second.first;

      if ( end <= start_itr ) {
        internal_map_.insert( { start, std::make_pair( data, is_last_substring ) } );
        inserted = true;
        break;
      } else if ( start < start_itr && start_itr < end && end <= end_itr ) {
        data = data.substr( 0, start_itr - start );
        internal_map_.insert( { start, std::make_pair( data, is_last_substring ) } );
        inserted = true;
        break;
      } else if ( start < start_itr && end_itr <= end ) {
        internal_map_.erase( itr++ );
        erased = true;
      } else if ( start_itr <= start && end <= end_itr ) {
        inserted = true;
        break;
      } else if ( start_itr <= start && start < end_itr && end_itr < end ) {
        data = data.substr( end_itr - start );
        start = end_itr;
      }

      if ( erased == false ) {
        itr++;
      } else {
        erased = false;
      }
    }

    if ( inserted == false ) {
      internal_map_.insert( { start, std::make_pair( data, is_last_substring ) } );
    }
  }

  // 顺序遍历, 写入到输出流
  for ( auto itr = internal_map_.begin(); itr != internal_map_.end(); ) {
    if ( itr->first == first_unassembled ) {
      output.push( itr->second.first );
      if ( itr->second.second ) {
        output.close();
      }
      first_unassembled += itr->second.first.size();
      internal_map_.erase( itr++ );
    } else {
      break;
    }
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  uint64_t nbytes = 0;
  for ( auto ele : internal_map_ ) {
    nbytes += ele.second.first.size();
  }
  return nbytes;
}
