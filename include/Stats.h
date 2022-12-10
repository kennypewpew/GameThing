#pragma once

enum SupportedStats {
    INVALID
  , HP
  , STR
  , DEX
  , CON
  , INT
  , WIS
  , CHA
  , N_STATS
};

//enum SupportedStats {
//    INVALID
//  , HP
//  , STR
//  , SPD
//  , DEF
//  , RES
//  , DEX
//  , LCK
//  , N_STATS
//};

class Stat {
 public:
  std::string name;
  int value;
};

class Skill {
 public:
  std::string name;
  uint64_t xp;

  bool AddXP(uint64_t &amount) {
    uint64_t tmp = xp + amount;
    if ( tmp > xp ) xp = tmp;
  }
};

class StatSheet {
 public:
  std::vector<Stat>

};

