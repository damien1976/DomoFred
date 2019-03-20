/*
 * EventStore.h
 *
 *  Created on: Jan 21, 2019
 *      Author: fmetz
 */

#ifndef EVENTSTORE_H_
#define EVENTSTORE_H_
#include <string.h>
#include "Event.h"

namespace fred {

class EventStore {

  public:

  EventStore();
  virtual ~EventStore();

  Event* getNextEventAndRemoveFromStore() {
    if (!eventList[idxFirstEvent]) {
      return 0;
    }

    Event *cur = eventList[idxFirstEvent];
    eventList[idxFirstEvent] = 0;
    calculateNextFirstEvent();
    return cur;
  }

  Event* setEvent(Event* e) {
    if (eventList[idxNextFreeSlot]) {
      // XXX FIXME List is full
      return 0;
    }
    eventList[idxNextFreeSlot] = e;
    calculateNextSlot();
    return e;
  }

  bool hasEvents() {
    return !(idxNextFreeSlot==idxFirstEvent);
  }

  unsigned int count() {
    unsigned int cnt=0;
    for (int idx=0; idx<MAXEVENTS; ++idx) {
      if (eventList[idx]) {
        ++cnt;
      }
    }
    return cnt;
  }

  void deleteEvent(Event* e) {
    if (e->nature)
      delete(e->nature);
    delete(e);
  }

  private:

  static const unsigned int MAXEVENTS = 32;

  Event* eventList[MAXEVENTS];
  int idxFirstEvent;
  int idxNextFreeSlot;

  // moves the idx to the next slot
  void calculateNextSlot() {
    ++idxNextFreeSlot;
    if (idxNextFreeSlot == MAXEVENTS) {
      idxNextFreeSlot = 0;
    } else if (idxNextFreeSlot > MAXEVENTS) {
      // XXX FIXME ERROR
    }
  }

  void calculateNextFirstEvent() {
    ++idxFirstEvent;
    if (idxFirstEvent == MAXEVENTS) {
      idxFirstEvent = 0;
    } else if (idxFirstEvent > MAXEVENTS) {
      // XXX FIXE ERROR
    }
  }

};
} /* namespace fred */

#endif /* EVENTSTORE_H_ */
