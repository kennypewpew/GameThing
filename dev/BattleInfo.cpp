#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include <functional>

#include "BattleEventQueue.h"

const int defaultCharacterQueueEntries = 2;

class BattleObject {
 public:
  int id;
  int coords[3];
  bool isActive;
  int standOnHeight;
  std::string tooltip;
  // graphics info - vertices, indices, texture?
};

enum DurationType {
    INVALID_DURATION_TYPE
  , UNIT_TURNS
  , ABSOLUTE_TICKS
  , DURATION_NONE
  , DURATION_TYPE_TOTAL
};

enum StatusType {
    INVALID_STATUS
  , STAT_DAMAGE
  , STAT_HEAL
  , STAT_DRAIN
  , MODIFY_INCOMING_DAMAGE
  , MODIFY_OUTGOING_DAMAGE
  , MODIFY_RESISTANCE
  , INVULNERABLE
  , STOPPED
  , MOVE_TOWARDS
  , DONT_ACT
  , DONT_MOVE
  , SLEEP
  , CHARM
  , BERSERK
  , BLIND
  , RERAISE
  , REVERSE_HEALING
  , UNTARGETABLE
  , STATUS_TYPE_TOTAL
};

class StatusEffect {
 public:
  StatusType name;
  DurationType durationType;
  int remainingTicks;
  float magnitude;
  float moveTowards[3];
};

class AttackMove {
 public:
  std::string name;
  std::string desc;
  std::vector<DurationType> effects;
  std::vector<std::pair<std::string,int>> atkWeights;
  std::vector<std::pair<std::string,int>> defWeights;
  std::string targetDmg;
};

enum SupportedStats {
    UNSUPPORTED_STAT
  , HP
  , MP
  , SPEED
  , MOVE
  , JUMP
  , PHYSICAL_POWER
  , MAGICAL_POWER
  , PHYSICAL_DEFENSE
  , MAGICAL_DEFENSE
  , BLOCK
  , EVADE
  , SUPPORTED_STATS_TOTAL
};

std::vector<std::string> SupportedStatsString {
    "UNSUPPORTED_STAT"
  , "HP"
  , "MP"
  , "SPEED"
  , "MOVE"
  , "JUMP"
  , "PHYSICAL_POWER"
  , "MAGICAL_POWER"
  , "PHYSICAL_DEFENSE"
  , "MAGICAL_DEFENSE"
  , "BLOCK"
  , "EVADE"
  , "SUPPORTED_STATS_TOTAL"
};
typedef int StatsContainer[SUPPORTED_STATS_TOTAL];

class BattleActor : public BattleObject {
 public:
  int turn = 0;
  int turnCounter;
  bool hasMoved, hasActed;
  StatsContainer ref_stats = {0};
  StatsContainer stat_mods = {0};
  std::vector<std::vector<AttackMove>> moveList;
  std::vector<StatusEffect> status;
  //std::vector<function pointer> startTurnHooks;
  //std::vector<function pointer> endTurnHooks;

  int MaxStat( const SupportedStats &stat ) {
    return (this->ref_stats[stat] - this->stat_mods[stat]);
  }
  int CurrentStat( const SupportedStats &stat ) const {
    return (ref_stats[stat] - stat_mods[stat]);
  }
  void MoveTo(const int *newCoords) {
    for ( int i = 0 ; i < 3 ; ++i ) this->coords[i] = newCoords[i];
  }
  void DeActivate() {
    this->isActive = false;
  }
  void ReActivate() {
    this->isActive = true;
  }
  void StartTurn() {
    hasMoved = false;
    hasActed = false;
    //for ( size_t i = 0 ; i < startTurnHooks ; ++i ) startTurnHooks[i]();
//std::cout << "Starting turn: " << this->id << "\n";
++this->turn;
printf("%d is starting turn #%d\n", this->id , this->turn);
  }
  void EndTurn() {
    //for ( size_t i = 0 ; i < startTurnHooks ; ++i ) endTurnHooks[i]();
  }
  void DamageStat( const SupportedStats &stat , const int &dmg ) {
    int tmp = this->stat_mods[stat] - dmg;
    if ( this->ref_stats[stat] + tmp < 0 ) tmp = -ref_stats[stat];
    this->stat_mods[stat] = tmp;
  }
  void HealStat( const SupportedStats &stat , const int &heal ) {
    int tmp = this->stat_mods[stat] + heal;
    if ( tmp > 0 ) tmp = 0;
    this->stat_mods[stat] = tmp;
  }
  void Tick() {
  }
};

class Environment : public BattleObject {
 public:
  bool interactable;
  // function_pointer interactHook
  void Interact() {
    //interactHook();
  }
};

class BattleInfo {
 public:
  std::vector<BattleObject> things;
};

void PrintActor( const BattleActor &act ) {
  for ( int i = 1 ; i < SUPPORTED_STATS_TOTAL ; ++i ) {
    printf("\t%-20s:\t%d + %d\n",SupportedStatsString[i].c_str(),act.ref_stats[i],act.stat_mods[i]);
  }
}

void TimeTick( std::vector<BattleActor> &actors ) {
  for ( size_t i = 0 ; i < actors.size() ; ++i ) {
    actors[i].Tick();
  }
}

void StartTurnHelper( void *a ) {
  BattleActor *b = (BattleActor*)a;
  b->StartTurn();
}

int AttackPreviewDamage( const BattleActor &atk , const BattleActor &def , const std::string &move ) {

  return 0;
}

void FillQueue( std::vector<BattleActor> &actors , EventQueue &q , const int &ticks = 0 ) {
  for ( size_t i = 0 ; i < actors.size() ; ++i ) {
    int t = ticks;
    for ( int j = 0 ; j < defaultCharacterQueueEntries ; ++j ) {
      int ticksUntil = TicksUntilTurn( actors[i].CurrentStat( SPEED ) , t );
      q.AddEvent( actors[i].id , t + ticksUntil , StartTurnHelper , &actors[i] );
      t += ticksUntil;
    }
  }
}


int main(void) {
  std::vector<BattleActor> actors(2);
  for ( int i = 1 ; i < SUPPORTED_STATS_TOTAL ; ++i ) {
    actors[0].ref_stats[i] = 10*i;
  }
  actors[0].DamageStat(HP,14);
  actors[0].HealStat(HP,4);
  actors[0].id = 3;
  PrintActor(actors[0]);

  for ( int i = 1 ; i < SUPPORTED_STATS_TOTAL ; ++i ) {
    actors[1].ref_stats[i] = 30*i;
  }
  actors[1].DamageStat(HP,14);
  actors[1].HealStat(HP,4);
  actors[1].id = 7;
  PrintActor(actors[1]);

  EventQueue q;

  FillQueue( actors , q , 0 );

  q.Eval();
  q.Eval();
  q.Eval();
  q.Eval();
  q.Eval();
  q.Eval();
  q.Eval();
  q.Eval();
  q.Eval();
  q.Eval();
  q.Eval();
  q.Eval();
}

