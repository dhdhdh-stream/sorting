#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <map>
#include <vector>

#include "run_helper.h"

class HiddenState;

const int EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int EXPERIMENT_STATE_TRAIN = 1;
const int EXPERIMENT_STATE_MEASURE = 2;
const int EXPERIMENT_STATE_VERIFY_EXISTING = 3;
const int EXPERIMENT_STATE_VERIFY = 4;

const int EXPERIMENT_STATE_SUCCESS = 5;
const int EXPERIMENT_STATE_FAIL = 6;

class Experiment {
public:
	HiddenState* parent;

	int starting_action;
	std::vector<HiddenState*> experiment_states;
	/**
	 * - starting is experiment_states[0]
	 */

	double average_remaining_experiments_from_start;

	int state;
	int state_iter;

	double existing_average_misguess;
	double existing_misguess_variance;

	/**
	 * - for new experiment_states only
	 */
	std::map<HiddenState*, std::vector<double>> ending_state_vals;
	std::map<HiddenState*, double> ending_state_means;

	double new_misguess;

	std::vector<double> misguess_histories;

	Experiment(HiddenState* parent,
			   int starting_action,
			   std::vector<HiddenState*> experiment_states);
	~Experiment();

	bool activate(HiddenState* curr_state,
				  std::vector<int>& action_sequence,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  HiddenState* ending_state);

	void measure_existing_backprop(double target_val,
								   HiddenState* ending_state);

	void train_activate(HiddenState* curr_state,
						std::vector<int>& action_sequence);
	void train_backprop(double target_val,
						HiddenState* ending_state);

	void measure_activate(HiddenState* curr_state,
						  std::vector<int>& action_sequence);
	void measure_backprop(double target_val,
						  HiddenState* ending_state);

	void verify_existing_backprop(double target_val,
								  HiddenState* ending_state);

	void verify_activate(HiddenState* curr_state,
						 std::vector<int>& action_sequence);
	void verify_backprop(double target_val,
						 HiddenState* ending_state);
};

#endif /* EXPERIMENT_H */