/*
 * NatureBase.h
 *
 *  Created on: Jan 21, 2019
 *      Author: fmetz
 */

#ifndef NATUREBASE_H_
#define NATUREBASE_H_

namespace fred {

enum NatureType {
	NONATURE = 0,
	PININPUT = 1,
	TEMPERATURE = 2
};


class NatureBase {
public:
	NatureBase();
	virtual ~NatureBase();
	NatureType natureType;
};

} /* namespace fred */

#endif /* NATUREBASE_H_ */
