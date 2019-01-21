/*
 * NaturePin.h
 *
 *  Created on: Jan 21, 2019
 *      Author: fmetz
 */

#ifndef NATUREPIN_H_
#define NATUREPIN_H_

#include "NatureBase.h"

namespace fred {

class NaturePin : public NatureBase {
  public:
	NaturePin();
	virtual ~NaturePin();
  char pinNumber;
};

} /* namespace fred */

#endif /* NATUREPIN_H_ */
