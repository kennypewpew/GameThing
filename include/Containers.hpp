#pragma once

#include <map>

template<typename T,typename uuid_t = uint32_t>
class UuidMapper {
 public:
  std::map<uuid_t,T> map;
  uuid_t cntr;

  UuidMapper() : cntr(0) {}

  uuid_t Insert( const T &toAdd ) {
    do { ++cntr; } while ( map.count( cntr ) || 0 == cntr );
    map[cntr] = toAdd;
    return cntr;
  }

  void Delete( const uuid_t id ) {
    map.erase(id);
  }
};


