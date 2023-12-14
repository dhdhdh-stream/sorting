#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include <vector>

#include "run_helper.h"

class HiddenState;

const int EXPERIMENT_TYPE_EXPERIMENT = 0;
const int EXPERIMENT_TYPE_CONNECTION = 1;

const int EXPERIMENT_RESULT_NA = 0;
const int EXPERIMENT_RESULT_SUCCESS = 1;
const int EXPERIMENT_RESULT_FAIL = 2;

class AbstractExperiment {
public:
	int type;

	HiddenState* parent;

	double average_remaining_experiments_from_start;

	int result;

	virtual ~AbstractExperiment() {};

	virtual void activate(HiddenState*& curr_state,
						  std::vector<int>& action_sequence,
						  RunHelper& run_helper) = 0;
	virtual void backprop(double target_val,
						  HiddenState* ending_state) = 0;
};

#endif /* ABSTRACT_EXPERIMENT_H */