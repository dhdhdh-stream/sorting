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

const int EXPERIMENT_STATE_RAMP = 0;
const int EXPERIMENT_STATE_MEASURE = 1;

const int MEASURE_STATUS_N_A = 0;
const int MEASURE_STATUS_SUCCESS = 1;
const int MEASURE_STATUS_FAIL = 2;

void init_experiment_helper(AbstractNode* node_context,
							bool is_branch,
							Wrapper* wrapper);

class ExperimentHistory;
class Experiment : public AbstractExperiment {
public:
	AbstractNode* node_context;
	bool is_branch;

	int state;
	int state_iter;

	std::vector<int> actions;
	AbstractNode* exit_next_node;

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

	int sum_original_num_instances;
	int sum_branch_num_instances;
	double original_average_instances_per_run;
	double branch_average_instances_per_run;

	bool is_force;

	Experiment();
	~Experiment();

	void experiment_activate(ExperimentRun* run);
	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);
	void predict_activate(PredictRun* run);
	void backprop(double target_val,
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