#pragma once
#include <cstddef>

int TicksUntilTurn( const int &speed , const int &currentCount );

class EventQueueEntry {
 public:
  void (*eventFn)(void*);
  void *eventObj = NULL;
  int tickCount;
  int idAffected;
  EventQueueEntry *next = NULL;
  EventQueueEntry *prev = NULL;

  EventQueueEntry() {}
  EventQueueEntry( EventQueueEntry *n , EventQueueEntry *p ) {}

  // TODO: Consistent tiebreaker if turn counts are equal
  void Insert( EventQueueEntry *e );
  void Delete( const int &id );
};

class EventQueue {
 public:
  EventQueueEntry *current;
  EventQueueEntry *events;
  EventQueue();

  static void DoNothing(void *) {}

  void AddEvent( const int &idTarget , const int &tickHappens , void (*fn)(void *) , void *objCall = NULL );

  void DeleteEvent( const int &id );
  void Eval();
  ~EventQueue();
};


