#pragma once

#include <vector>
#include <map>

// Define stats/skills here and the become enum + string
#define STAT_IMPL    \
    S_IMPL(STAT_INVALID)  \
  , S_IMPL(HP )      \
  , S_IMPL(STR)      \
  , S_IMPL(DEX)      \
  , S_IMPL(CON)      \
  , S_IMPL(INT)      \
  , S_IMPL(WIS)      \
  , S_IMPL(CHA)      \
  , S_IMPL(N_STATS)

// Ideally make these all one word skills (eg. lightarmor -> leather)
// Also sort by combat, overworld, etc
//   I don't think it's worth making separate structures for each skill category
#define SKILL_IMPL     \
    S_IMPL(SKILL_INVALID)    \
  , S_IMPL(SNEAK)      \
  , S_IMPL(LOCKPICK)   \
  , S_IMPL(PERSUASION) \
  , S_IMPL(SWORDS)     \
  , S_IMPL(LIGHTARMOR) \
  , S_IMPL(N_SKILLS)

#define GEAR_IMPL   \
    S_IMPL(GEAR_INVALID) \
  , S_IMPL(WEAPON_1)\
  , S_IMPL(WEAPON_2)\
  , S_IMPL(HEAD)    \
  , S_IMPL(BODY)    \
  , S_IMPL(LEGS)    \
  , S_IMPL(FEET)    \
  , S_IMPL(ARMS)    \
  , S_IMPL(HANDS)   \
  , S_IMPL(BACK)    \
  , S_IMPL(ACC_1)   \
  , S_IMPL(ACC_2)   \
  , S_IMPL(ACC_3)   \
  , S_IMPL(ACC_4)   \
  , S_IMPL(N_GEAR)

#define MOD_IMPL    \
    S_IMPL(MOD_INVALID) \
  , S_IMPL(SHARP)   \
  , S_IMPL(DULL)    \
  , S_IMPL(FLAMING) \
  , S_IMPL(LIGHT)   \
  , S_IMPL(CURSED)  \
  , S_IMPL(N_MODS)

#define ITEM_IMPL    \
    S_IMPL(ITEM_INVALID) \
  , S_IMPL(WEAPON)    \
  , S_IMPL(ARMOR)     \
  , S_IMPL(HELMET)    \
  , S_IMPL(GREAVES)   \
  , S_IMPL(BOOTS)     \
  , S_IMPL(BRACERS)   \
  , S_IMPL(GLOVES)    \
  , S_IMPL(CLOAK)     \
  , S_IMPL(RING)      \
  , S_IMPL(NECKLACE)  \
  , S_IMPL(N_ITEMS)

#define TRAIT_IMPL    \
    S_IMPL(TRAIT_INVALID) \
  , S_IMPL(PATIENT)    \
  , S_IMPL(GENEROUS)     \
  , S_IMPL(MERCIFUL)    \
  , S_IMPL(VINDICTIVE)   \
  , S_IMPL(HONOURABLE)     \
  , S_IMPL(LOYAL)   \
  , S_IMPL(N_TRAITS)

// Simplify to bare tasks that can be chained together for jobs
#define TASK_IMPL      \
    S_IMPL(TASK_NONE)  \
  , S_IMPL(FETCH)      \
  , S_IMPL(DELIVER)    \
  , S_IMPL(GOHERE)     \
  , S_IMPL(INTERCEPT)  \
  , S_IMPL(ATTACK)     \
  , S_IMPL(DEFEND)     \
  , S_IMPL(DISCOVER)   \
  , S_IMPL(SCOUT)      \
  , S_IMPL(EXPLORE)    \
  , S_IMPL(BUILD)      \
  , S_IMPL(WAIT)       \
  , S_IMPL(N_TASKS)


#define MAX_GEAR_MODS 3

#define S_IMPL(s) s
enum SupportedStats  { STAT_IMPL  };
enum SupportedSkills { SKILL_IMPL };
enum SupportedGear   { GEAR_IMPL  };
enum SupportedMods   { MOD_IMPL   };
enum ItemTypes       { ITEM_IMPL  };
enum TaskType        { TASK_IMPL  };
#undef S_IMPL


// So that we can keep the enum and names next to each other
#ifdef COMPILE_STATS
  #define EXTERN
  #define IMPLEMENT(a)\
    [] = { a };
#else
  #define EXTERN extern
  #define IMPLEMENT(a) []
#endif

#define S_IMPL(s) #s
EXTERN const char *StatNames  IMPLEMENT( STAT_IMPL  );
EXTERN const char *SkillNames IMPLEMENT( SKILL_IMPL );
EXTERN const char *GearNames  IMPLEMENT( GEAR_IMPL  );
EXTERN const char *ModNames   IMPLEMENT( MOD_IMPL   );
EXTERN const char *ItemNames  IMPLEMENT( ITEM_IMPL  );
EXTERN const char *TaskNames  IMPLEMENT( TASK_IMPL  );

#undef S_IMPL
#ifndef COMPILE_STATS
#undef EXTERN
#endif

typedef int StatSheet[N_STATS];
typedef uint64_t SkillXp[N_SKILLS];
typedef uint64_t Gear[N_GEAR];

typedef uint16_t ItemId;

class Item {
 public:
  ItemId uid;
  std::string name;
  uint8_t weight;
  virtual ItemTypes Type() = 0;
};

class Weapon : public Item {
 public:
  uint8_t range;
  uint8_t damage;
  //WeaponType type;
};

class Armor : public Item {
 public:
  uint8_t pdef;
  uint8_t mdef;
  //ArmorType type;
};

struct Equippable {
  ItemId baseItem;
  SupportedMods mods[MAX_GEAR_MODS];
};

void AddXp( SkillXp xp, SupportedSkills field, int val );

class Adventurer {
 public:
  StatSheet stats;
  SkillXp skills;
  Gear equipped;
};

class Inventory {

};



