/**
 * TODO: special case experiment from EndNode
 */

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class ExperimentRun;
class Network;
class PredictRun;
class Wrapper;

const int EXPERIMENT_STATE_GATHER = 0;
const int EXPERIMENT_STATE_RAMP = 1;
const int EXPERIMENT_STATE_MEASURE = 2;

const int MEASURE_STATUS_N_A = 0;
const int MEASURE_STATUS_SUCCESS = 1;
const int MEASURE_STATUS_FAIL = 2;

class ExperimentHistory;
class Experiment : public AbstractExperiment {
public:
	AbstractNode* node_context;
	bool is_branch;
	AbstractNode* exit_next_node;

	int state;
	int state_iter;

	std::vector<std::vector<double>> start_state_history;
	std::vector<double> start_target_val_history;
	int start_hits;
	std::vector<std::vector<double>> end_state_history;
	std::vector<double> end_target_val_history;
	int end_hits;

	std::vector<int> actions;

	Network* original_network;
	Network* branch_network;

	int starting_iter;

	int total_count;
	double existing_sum_scores;
	int existing_count;
	double new_sum_scores;
	int new_count;

	int curr_ramp;
	/**
	 * - simply init to 0 and fast fail
	 */
	int measure_status;

	double predicted_local_improvement;
	double predicted_global_improvement;
	double local_improvement;
	double global_improvement;

	Experiment(AbstractNode* node_context,
			   bool is_branch,
			   AbstractNode* exit_next_node,
			   Wrapper* wrapper);
	~Experiment();

	void experiment_activate(ExperimentRun* run);
	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);
	void experiment_exit(ExperimentRun* run);
	void backprop(double target_val,
				  ExperimentHistory* history,
				  Wrapper* wrapper);

	void gather_activate(ExperimentRun* run);
	void gather_exit(ExperimentRun* run);
	void gather_backprop(double target_val,
						 ExperimentHistory* history,
						 Wrapper* wrapper);

	void ramp_activate(ExperimentRun* run);
	void ramp_step(int& action,
				   bool& is_next,
				   ExperimentRun* run);
	void ramp_backprop(double target_val,
					   ExperimentHistory* history,
					   Wrapper* wrapper);

	void pad_new_state(int num_add);

	void add(Wrapper* wrapper);
};

class ExperimentHistory {
public:
	bool is_on;

	bool hit_branch;

	ExperimentHistory(Experiment* experiment);
};

class ExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExperimentState(Experiment* experiment);
};

#endif /* EXPERIMENT_H */