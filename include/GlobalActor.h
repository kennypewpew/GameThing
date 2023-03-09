#pragma once

#include "Types.h"
#include "Containers.hpp"
#include "Stats.h"

#include <queue>
#include <memory>

extern const int MAPXDIM;
extern const int MAPYDIM;

class GlobalActor {
 public:
  size_t uid;
  std::map<size_t,float> relations;

  GlobalActor() {}
  GlobalActor(size_t id) : uid(id) {}
  ~GlobalActor() {}

  void DecayRelations(float rate = 0.95);
  void ModifyRelation(size_t with, float change);
  virtual void TakeAction() = 0;
};

enum GlobalActorType {
    GLOBAL_ACTOR_INVALID
  , GLOBAL_ACTOR_FIXED
  , GLOBAL_ACTOR_MOBILE
  , GLOBAL_ACTOR_CONTAINER
  , N_GLOBAL_ACTOR
};

class GlobalActorTracker {
 public:
  UuidMapper<std::shared_ptr<GlobalActor>,size_t> allGlobalActors; 
  GlobalActorType type;

  GlobalActorTracker() {}
  ~GlobalActorTracker() {}

  template<typename T, class... Args>
  std::shared_ptr<T> CreateActor(Args&&... args) {
    std::shared_ptr<T> ret = std::make_shared<T>(args...);
    size_t id = allGlobalActors.Insert(ret);
    ret->uid = id;
    return ret;
  }

  void RemoveActor(size_t id);

  GlobalActorType Type();

  void Act();

  template<typename T>
  T* Ptr(size_t id) {
    return (T*)(allGlobalActors.map[id].get());
  }
};

extern GlobalActorTracker actors;

class City : public GlobalActor {
 public:
  std::string name;
  size_t population;
  float wealth;
  float security;
  float happiness;
  // disposition

  City() : wealth(10.) , security(3.) , happiness(50.) {}

  void UpdateHappiness();

  void TakeAction();
};

class Task {
 public:
  Pos2D loc;
  TaskType type;
  float difficulty;
  size_t targetUid;
  int duration;

  Task() : type(TASK_NONE) {}
  Task( TaskType t , Pos2D l , float d , int dur = 0 )
      : type(t), loc(l), difficulty(d), targetUid(0), duration(dur) {}
  Task( TaskType t , size_t i , float d , int dur = 0 )
      : type(t), targetUid(i), difficulty(d), loc({-1,-1}), duration(dur) {}
};

class Quest {
 public:
  std::queue<Task> steps;

  Task& CurrentStep();
  void StepComplete();
  void AddStep(Task step);
};

class Party : public GlobalActor {
 public:
  std::vector<std::shared_ptr<Adventurer>> members;
  Inventory inventory;
  Quest activeQuest;
  Quest longerTermQuest;
  size_t parentActorId;
  Pos2Df location;
  bool hidden;

  void TakeAction();
  bool ReactToWorld();
  bool MoveTowards(Pos2D p );
  bool MoveTowards(Pos2Df p);
  void ContinueTask();
  void Die();

  float Speed();
  float PowerLevel();
  float VisionRange();
};


