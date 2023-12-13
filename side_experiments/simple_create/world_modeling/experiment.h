// - world model can't pay attention to observations because won't be focused?
//   - unless experiments train world model and solution jointly

// - on solution experiment, also create cancel world model experiment?

// - how do obs/state line up/get shared between world model and solution?

// - in solutions, actions have an effect on state in specific contexts
//   - how can this align with world model?
//     - actions will be performed out-of-context, so can't affect state?
//       - so maybe don't share states?

// - compound actions depend on state given to them
//   - so can't model compound actions without taking into account state

// - maybe world model matches solution, except has extra for when going off track?

// - perhaps simply predict obs
//   - and treat raw actions as is

// - then for compound actions, learn custom

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <map>
#include <vector>

#include "abstract_experiment.h"
#include "run_helper.h"

class HiddenState;

const int EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int EXPERIMENT_STATE_TRAIN = 1;
const int EXPERIMENT_STATE_MEASURE = 2;
const int EXPERIMENT_STATE_VERIFY_EXISTING = 3;
const int EXPERIMENT_STATE_VERIFY = 4;

class Experiment : public AbstractExperiment {
public:
	std::vector<HiddenState*> experiment_states;
	/**
	 * - starting is experiment_states[0]
	 */

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
			   std::vector<HiddenState*> experiment_states);
	~Experiment();

	void activate(HiddenState*& curr_state,
				  std::vector<int>& action_sequence,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  HiddenState* ending_state);

	void measure_existing_backprop(double target_val,
								   HiddenState* ending_state);

	void train_activate(HiddenState*& curr_state,
						std::vector<int>& action_sequence);
	void train_backprop(double target_val,
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

#endif /* EXPERIMENT_H */