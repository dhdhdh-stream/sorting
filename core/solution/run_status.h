#ifndef RUN_STATUS_H
#define RUN_STATUS_H

#include "constants.h"

class RunStatus {
public:
	int curr_depth;
	int max_depth;
	bool exceeded_depth;

	int explore_phase;
	double existing_score;

	RunStatus() {
		this->curr_depth = 0;
		this->max_depth = 0;
		this->exceeded_depth = false;

		this->explore_phase = EXPLORE_TYPE_NONE;
		// setting this->existing_score doesn't matter
	}
};

#endif /* EXPLORE_STATUS_H */