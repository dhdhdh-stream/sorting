#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include <vector>

#include "run_helper.h"

class Action;
class WorldState;

const int EXPERIMENT_TYPE_EXPERIMENT = 0;
const int EXPERIMENT_TYPE_CONNECTION = 1;

const int EXPERIMENT_RESULT_NA = 0;
const int EXPERIMENT_RESULT_SUCCESS = 1;
const int EXPERIMENT_RESULT_FAIL = 2;

class AbstractExperiment {
public:
	int type;

	WorldState* parent;
	bool is_obs;
	int obs_index;
	bool obs_is_greater;
	Action* action;

	double average_remaining_experiments_from_start;

	int result;

	virtual ~AbstractExperiment() {};

	virtual bool activate(WorldState*& curr_state,
						  RunHelper& run_helper) = 0;
	virtual void backprop(double target_val,
						  WorldState* ending_state,
						  std::vector<double>& ending_state_vals) = 0;
};

#endif /* ABSTRACT_EXPERIMENT_H */