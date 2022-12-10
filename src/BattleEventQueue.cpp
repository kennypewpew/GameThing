#include "BattleEventQueue.h"
#include <cstdio>
#include <cstddef>

const int turnReached = 1000;

int TicksUntilTurn( const int &speed , const int &currentCount ) {
  if ( currentCount > turnReached ) return 0;
  int rem = turnReached - currentCount;
  if ( rem % turnReached ) return ( (rem / speed) + 1 );
  else                     return   (rem / speed)      ;
}

//class EventQueueEntry {
// public:
//  void (*eventFn)(void*);
//  void *eventObj;
//  int tickCount;
//  int idAffected;
//  EventQueueEntry *next;
//  EventQueueEntry *prev;

  //EventQueueEntry::EventQueueEntry() {}
  //EventQueueEntry::EventQueueEntry( EventQueueEntry *n , EventQueueEntry *p ) {}

  //void Init() {
  //}

  // TODO: Consistent tiebreaker if turn counts are equal
  void EventQueueEntry::Insert( EventQueueEntry *e ) {
    if ( e->tickCount > this->tickCount ) {
      if ( NULL != this->next ) {
        this->next->Insert(e);
      }
      else {
	e->next = this->next;
        e->prev = this;
	this->next = e;
      }
    }
    else {
      if ( NULL != this->prev ) {
        this->prev->next = e;
        e->prev = this->prev;
        e->next = this;
        this->prev = e;
      }
      else {
        this->prev = e;
        e->next = this;
      }
    }
  }
  void EventQueueEntry::Delete( const int &id ) {
    if ( NULL != this->next ) this->next->Delete( id );
    if ( id == this->idAffected ) {
      if ( NULL != this->next ) {
        this->next->prev = this->prev;
        this->prev->next = this->next;
      }
      else {
        this->prev->next = NULL;
      }
      delete this;
    }
  }
//};

void EvalEvent( EventQueueEntry *curr ) {
  if ( NULL == curr->eventObj ) {
    curr->eventFn(NULL);
  }
  else {
    curr->eventFn(curr->eventObj);
  }
}


  EventQueue::EventQueue() {
    EventQueueEntry *start = new EventQueueEntry;
    EventQueueEntry *end   = new EventQueueEntry;

    start->idAffected = -1;
    start->tickCount = -1;
    start->eventFn = this->DoNothing;

    end->idAffected = -1;
    end->tickCount = 1 << 30;
    end->eventFn = this->DoNothing;

    this->events = start;
    this->current = start;
    this->current->Insert(end);
  }

  void EventQueue::AddEvent( const int &idTarget , const int &tickHappens , void (*fn)(void *) , void *objCall ) {
printf("Adding event from %d at tick %d\n",idTarget,tickHappens);
    EventQueueEntry *q = new EventQueueEntry;
    q->idAffected = idTarget;
    q->tickCount = tickHappens;
    q->eventFn = fn;
    q->eventObj = objCall;

    if ( NULL == events ) events = q;
    else events->Insert(q);
  }

  void EventQueue::DeleteEvent( const int &id ) {
    this->events->Delete(id);
  }
  void EventQueue::Eval() {
    if ( NULL == current ) this->current = this->events;
    if ( NULL == current->next ) return;
    EvalEvent(this->current);
    this->current = this->current->next;
  }
  EventQueue::~EventQueue() {
  }


