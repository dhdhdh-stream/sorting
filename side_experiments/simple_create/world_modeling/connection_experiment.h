#ifndef CONNECTION_EXPERIMENT_H
#define CONNECTION_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "run_helper.h"

class HiddenState;

const int CONNECTION_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int CONNECTION_EXPERIMENT_STATE_MEASURE = 1;
const int CONNECTION_EXPERIMENT_STATE_VERIFY_EXISTING = 2;
const int CONNECTION_EXPERIMENT_STATE_VERIFY = 3;

class ConnectionExperiment : public AbstractExperiment {
public:
	HiddenState* target;

	int state;
	int state_iter;

	double existing_average_misguess;
	double existing_misguess_variance;

	double new_misguess;

	std::vector<double> misguess_histories;

	ConnectionExperiment(HiddenState* parent,
						 HiddenState* target);

	void activate(HiddenState*& curr_state,
				  std::vector<int>& action_sequence,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  HiddenState* ending_state);

	void measure_existing_backprop(double target_val,
								   HiddenState* ending_state);

	void measure_activate(HiddenState*& curr_state,
						  std::vector<int>& action_sequence);
	void measure_backprop(double target_val,
						  HiddenState* ending_state);

	void verify_existing_backprop(double target_val,
								  HiddenState* ending_state);

	void verify_activate(HiddenState*& curr_state,
						 std::vector<int>& action_sequence);
	void verify_backprop(double target_val,
						 HiddenState* ending_state);
};

#endif /* CONNECTION_EXPERIMENT_H */