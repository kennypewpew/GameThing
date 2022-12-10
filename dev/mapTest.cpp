#include <iostream>
#include <vector>

#include "Maps.h"
#include "MapGen.h"

void PrintMapPos( const Map &mp , const int &x , const int &y ) {
  for ( int i = 0 ; i < mp._ydim ; ++i ) {
    for ( int j = 0 ; j < mp._xdim ; ++j ) {
      if ( x != j || y != i ) printf("  %d",mp.h(j,i));
      else printf("  x");
    }
    printf("\n");
  }
  if ( mp._extras.size() ) {
    size_t next = 0;
    for ( int i = 0 ; i < mp._ydim ; ++i ) {
      for ( int j = 0 ; j < mp._xdim ; ++j ) {
        if ( next == mp._extras.size() ) printf("  .");
        else {
          if ( mp._extras[next].x == j && mp._extras[next].y == i ) {
            printf("  %d", mp._extras[next].height);
            ++next;
          }
          else printf("  .");
        }
      }
      printf("\n");
    }
  }
}

void SetReachable( const Map &mp , Map &buffer , const int &x , const int &y , const int &range ) {
  for ( int i = 0 ; i < mp._ydim * mp._xdim ; ++i ) {
    buffer._tiles[i].height = 0;
  }
  for ( size_t i = 0 ; i < mp._extras.size() ; ++i ) {
    buffer._extras[i].height = 0;
  }

  buffer.h(x,y) = range+1;
  bool todo = true;
  while ( todo ) {
    todo = false;
    for ( int i = 0 ; i < mp._ydim ; ++i ) {
      for ( int j = 0 ; j < mp._xdim ; ++j ) {
        if ( buffer.h(j,i) ) {
          int less = buffer.h(j,i) - 1;
          if ( buffer.InBounds(j+1,i+0) ) if ( buffer.h(j+1,i+0) < less ) { buffer.h(j+1,i+0) = less; todo = true; }
          if ( buffer.InBounds(j-1,i+0) ) if ( buffer.h(j-1,i+0) < less ) { buffer.h(j-1,i+0) = less; todo = true; }
          if ( buffer.InBounds(j-0,i+1) ) if ( buffer.h(j-0,i+1) < less ) { buffer.h(j-0,i+1) = less; todo = true; }
          if ( buffer.InBounds(j+0,i-1) ) if ( buffer.h(j+0,i-1) < less ) { buffer.h(j+0,i-1) = less; todo = true; }
        }
      }
    }
    //if ( mp._extras.size() ) {
    //  size_t next = 0;
    //  for ( int i = 0 ; i < mp._ydim ; ++i ) {
    //    for ( int j = 0 ; j < mp._xdim ; ++j ) {
    //      if ( next == mp._extras.size() ) printf("  .");
    //      else {
    //        if ( mp._extras[next].x == j && mp._extras[next].y == i ) {
    //          printf("  %d", mp._extras[next].height);
    //          ++next;
    //        }
    //        else printf("  .");
    //      }
    //    }
    //    printf("\n");
    //  }
    //}
  }
}

int main(void) {
  std::vector<VerticalityFeatures> f;
  f.push_back(FLAT);
  Map mp = GenerateMap( 7 , 5 , f );
  Map tmp = mp;
  printf("\n");
  PrintMapPos(mp,2,2);
  printf("\n");
  SetReachable( mp , tmp , 2 , 2 , 3 );
  PrintMap(tmp);
  printf("\n");
}
