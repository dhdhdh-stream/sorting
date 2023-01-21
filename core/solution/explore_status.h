#ifndef EXPLORE_STATUS_H
#define EXPLORE_STATUS_H

#include "definitions.h"

class ExploreStatus {
public:
	int explore_phase;
	double existing_score;

	ExploreStatus() {
		this->explore_phase = EXPLORE_TYPE_NONE;
		// setting this->existing_score doesn't matter
	}
};

#endif /* EXPLORE_STATUS_H */