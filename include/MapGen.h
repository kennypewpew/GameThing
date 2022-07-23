#pragma once

#include <vector>

#include <Maps.h>

enum VerticalityFeatures {
    FLAT
  , HILL
  , MOUNTAIN
  , ROCKS
  , CLIFF
  , PLATEAU
  , HOUSE
  , CITY
  , TOWN
  , RIVER
  , POND
  , VERTICAL_FEATURE_TOTAL
};

Map GenerateMap( const int &w , const int &l , const std::vector<VerticalityFeatures> &ft );
