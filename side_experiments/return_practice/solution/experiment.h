#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <vector>

#include "experiment_run.h"
#include "predict_run.h"

class AbstractNode;
class Network;
class Solution;
class WorldModelWrapper;

double predict_helper(AbstractNode* node_context,
					  bool is_branch,
					  std::vector<double>& starting_state,
					  WorldModelWrapper* wrapper);
double predict_helper(std::vector<int>& actions,
					  AbstractNode* exit_next_node,
					  std::vector<double>& starting_state,
					  WorldModelWrapper* wrapper);
void init_experiment_helper(AbstractNode* node_context,
							bool is_branch,
							Solution* solution,
							WorldModelWrapper* wrapper);

const int EXPERIMENT_STATE_RAMP = 0;
const int EXPERIMENT_STATE_MEASURE = 1;

const int MEASURE_STATUS_N_A = 0;
const int MEASURE_STATUS_SUCCESS = 1;
const int MEASURE_STATUS_FAIL = 2;

class Experiment {
public:
	AbstractNode* node_context;
	bool is_branch;

	std::vector<int> actions;
	AbstractNode* exit_next_node;

	std::vector<int> input_indexes;
	Network* original_network;
	Network* branch_network;

	int state;
	int state_iter;

	int starting_iter;

	std::vector<double> existing_scores;
	std::vector<double> new_scores;
	int measure_status;

	int curr_ramp;
	/**
	 * - simply init to 0 and fast fail
	 */

	double local_improvement;
	double global_improvement;



	void experiment_activate(ExperimentRun& run);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 ExperimentRun& run);

	void predict_activate(PredictRun& run);
	void predict_step(PredictRun& run);


};

class ExperimentHistory {
public:
	bool is_on;

	bool hit_branch;

	ExperimentHistory(Experiment* experiment);
};

class ExperimentState {
public:
	Experiment* experiment;

	int step_index;

	ExperimentState(Experiment* experiment);
};

#endif /* EXPERIMENT_H */