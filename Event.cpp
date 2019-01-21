/*
 * Event.cpp
 *
 *  Created on: Jan 21, 2019
 *      Author: fmetz
 */

#include "Event.h"

namespace fred {

Event::Event(): _e(NOEVENT), nature(0) {

}

Event::Event(EventType e): nature(0) {
  _e = e;
}

} /* namespace fred */
