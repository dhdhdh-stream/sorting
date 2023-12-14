#ifndef CONNECTION_EXPERIMENT_H
#define CONNECTION_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "run_helper.h"

class WorldState;

const int CONNECTION_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int CONNECTION_EXPERIMENT_STATE_MEASURE = 1;
const int CONNECTION_EXPERIMENT_STATE_VERIFY_EXISTING = 2;
const int CONNECTION_EXPERIMENT_STATE_VERIFY = 3;

class ConnectionExperiment : public AbstractExperiment {
public:
	WorldState* target;

	int state;
	int state_iter;

	double existing_average_misguess;
	double existing_misguess_variance;

	double new_misguess;

	std::vector<double> misguess_histories;

	ConnectionExperiment(WorldState* parent,
						 bool is_obs,
						 int obs_index,
						 bool obs_is_greater,
						 Action* action,
						 WorldState* target);

	bool activate(WorldState*& curr_state,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  WorldState* ending_state,
				  std::vector<double>& ending_state_vals);

	void measure_existing_backprop(double target_val,
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

#endif /* CONNECTION_EXPERIMENT_H */