#define COMPILE_STATS
#include "Stats.h"

void AddXp( SkillXp xp, SupportedSkills field, int val ) {
  if ( (xp[field] + val) > xp[field] ) xp[field] = xp[field] + val;
  // TODO: else saturate
}

