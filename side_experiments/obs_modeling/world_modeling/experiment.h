// TODO: test partial progress
// - i.e., finding spots that require a combination of obs/states

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <map>
#include <vector>

#include "abstract_experiment.h"
#include "run_helper.h"

class WorldState;

const int EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int EXPERIMENT_STATE_TRAIN = 1;
const int EXPERIMENT_STATE_MEASURE = 2;
const int EXPERIMENT_STATE_VERIFY_EXISTING = 3;
const int EXPERIMENT_STATE_VERIFY = 4;

class Experiment : public AbstractExperiment {
public:
	std::vector<WorldState*> experiment_states;
	/**
	 * - starting is experiment_states[0]
	 */

	int state;
	int state_iter;

	double existing_average_misguess;
	double existing_misguess_variance;

	std::map<WorldState*, int> experiment_state_reverse_mapping;
	std::vector<std::vector<double>> ending_vals;
	std::vector<std::vector<std::vector<double>>> ending_state_vals;
	std::vector<double> ending_val_averages;
	std::vector<std::vector<double>> ending_state_val_impacts;

	double new_misguess;

	std::vector<double> misguess_histories;

	Experiment(WorldState* parent,
			   bool is_obs,
			   int obs_index,
			   bool obs_is_greater,
			   Action* action,
			   std::vector<WorldState*> experiment_states);
	~Experiment();

	bool activate(WorldState*& curr_state,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  WorldState* ending_state,
				  std::vector<double>& ending_state_vals);

	void measure_existing_backprop(double target_val,
								   WorldState* ending_state,
								   std::vector<double>& ending_state_vals);

	void train_activate(WorldState*& curr_state);
	void train_backprop(double target_val,
						WorldState* ending_state,
						std::vector<double>& ending_state_vals);

	void measure_activate(WorldState*& curr_state);
	void measure_backprop(double target_val,
						  WorldState* ending_state,
						  std::vector<double>& ending_state_vals);

	void verify_existing_backprop(double target_val,
								  WorldState* ending_state,
								  std::vector<double>& ending_state_vals);

	void verify_activate(WorldState*& curr_state);
	void verify_backprop(double target_val,
						 WorldState* ending_state,
						 std::vector<double>& ending_state_vals);
};

#endif /* EXPERIMENT_H */