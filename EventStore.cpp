/*
 * EventStore.cpp
 *
 *  Created on: Jan 21, 2019
 *      Author: fmetz
 */

#include "EventStore.h"

namespace fred {

EventStore::EventStore(): idxFirstEvent(0), idxNextFreeSlot(0) {
	memset(&eventList,0,MAXEVENTS);
}

EventStore::~EventStore() {
	// TODO Auto-generated destructor stub
}

} /* namespace fred */
