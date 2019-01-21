/*
 * Event.h
 *
 *  Created on: Jan 21, 2019
 *      Author: fmetz
 */

#ifndef EVENT_H_
#define EVENT_H_

#include "NatureBase.h"

namespace fred {


enum EventType {
	NOEVENT = 0,
	ONRELEASE = 1,
	ONLONGPRESSDOWN = 2,
};

class Event {
public:
	Event::Event();

	Event::Event(EventType e);

	EventType _e;
	NatureBase* nature;

};
} /* namespace fred */

#endif /* EVENT_H_ */
