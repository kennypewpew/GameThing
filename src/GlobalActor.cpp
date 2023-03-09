#include "GlobalActor.h"

int MAPXDIM = 1;
int MAPYDIM = 1;

void SetMapDims(int x, int y) {
  MAPXDIM = x;
  MAPYDIM = y;
}

void GlobalActor::DecayRelations(float rate) {
  for ( auto &r : relations ) {
    r.second *= rate;
  }
}
void GlobalActor::ModifyRelation(size_t with, float change) {
  if ( !relations.count(with) ) relations[with] = 0.;
  relations[with] += change;
}

void GlobalActorTracker::RemoveActor(size_t id) {
  allGlobalActors.Delete(id);
}

GlobalActorType GlobalActorTracker::Type() {
  return type;
}

void GlobalActorTracker::Act() {
  for ( auto &a : allGlobalActors.map ) a.second->TakeAction();
}

// TODO:
GlobalActorTracker actors;

void City::UpdateHappiness() {
  happiness += 0.1 * (wealth * security - happiness);
}
void City::TakeAction() {
  UpdateHappiness();
  printf("City action:\n");
  printf("\t  w   |   s   |   h\n\t%1.3f | %1.3f | %1.3f\n", wealth, security, happiness);
}

Task& Quest::CurrentStep() {
  if ( !steps.size() ) steps.push(Task());
  return steps.front();
}
void Quest::StepComplete() {
  steps.pop();
}
void Quest::AddStep(Task step) {
  steps.push(step);
}

void Party::TakeAction() {
  printf("Party action:\n");
  // Check for nearby parties and act accordingly
  //   If no action needed, proceed with activeQuest

  bool reacted = ReactToWorld();
  if ( !reacted ) ContinueTask();

}

bool Party::ReactToWorld() {
  return false;
}

bool Party::MoveTowards(Pos2D p ) {
  Pos2Df pp = { float(p.x) , float(p.y) };
  return MoveTowards(pp);
}

bool Party::MoveTowards(Pos2Df p) {
  float xDiff = p.x - location.x;
  float yDiff = p.y - location.y;

  bool xReached = abs(xDiff) < 0.01;
  bool yReached = abs(yDiff) < 0.01;

  // Avoid division by zero
  float xProp = xReached ? 0 : xDiff / ( abs(xDiff) + abs(yDiff) );
  float yProp = yReached ? 0 : yDiff / ( abs(xDiff) + abs(yDiff) );

  location.x += Speed() * xProp;
  location.y += Speed() * yProp;

  float xDiffNew = p.x - location.x;
  float yDiffNew = p.y - location.y;

  // Avoid overshoot oscillations
  if ( xDiffNew * xDiff < 0 ) {
    location.x = p.x;
    xReached = true;
  }
  if ( yDiffNew * yDiff < 0 ) {
    location.y = p.y;
    yReached = true;
  }

  return xReached & yReached;
}

void Party::ContinueTask() {
  Task &task = activeQuest.CurrentStep();
  TaskType t = task.type;
  if ( TASK_NONE == t ) {
    // Decide a new task for yourself
    //   For now, think and then go somewhere random
    activeQuest.StepComplete();
    int d = rand() % 10;
    activeQuest.AddStep( Task( WAIT , {0,0} , 1. , d ) );
    int x = rand() % MAPXDIM;
    int y = rand() % MAPYDIM;
    activeQuest.AddStep( Task( GOHERE , {x,y} , 1. ) );
  }
  else if ( GOHERE    == t ) {
    bool reached = MoveTowards(task.loc);
    if ( reached ) activeQuest.StepComplete();
  }
  else if ( INTERCEPT  == t ) {
  }
  else if ( FETCH == t ) {
  }
  else if ( DELIVER   == t ) { }
  else if ( ATTACK    == t ) { }
  else if ( DEFEND    == t ) {
    actors.Ptr<City>(1)->security += PowerLevel();
  }
  else if ( DISCOVER  == t ) { }
  else if ( SCOUT     == t ) { }
  else if ( BUILD     == t ) { }
  else if ( WAIT      == t ) {
    --task.duration;
    if ( task.duration <= 0 ) activeQuest.StepComplete();
  }
  else {} // unsupported task
}

void Party::Die() {
  //if ( parentActorId )
  //actors[parentActorId].ModifyAccordingly();
}

float Party::Speed() { return 1.; }
float Party::PowerLevel() { return 1.; }
float Party::VisionRange() { return 10.; }

