#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <vector>

#include "experiment_run.h"
#include "predict_run.h"

class AbstractNode;
class Network;
class Wrapper;

double predict_helper(AbstractNode* node_context,
					  bool is_branch,
					  std::vector<double>& starting_state,
					  Wrapper* wrapper);
double predict_helper(std::vector<int>& actions,
					  AbstractNode* exit_next_node,
					  std::vector<double>& starting_state,
					  Wrapper* wrapper);
void init_experiment_helper(AbstractNode* node_context,
							bool is_branch,
							Wrapper* wrapper);

const int EXPERIMENT_STATE_RAMP = 0;
const int EXPERIMENT_STATE_MEASURE = 1;

const int MEASURE_STATUS_N_A = 0;
const int MEASURE_STATUS_SUCCESS = 1;
const int MEASURE_STATUS_FAIL = 2;

class ExperimentHistory;
class Experiment {
public:
	AbstractNode* node_context;
	bool is_branch;

	std::vector<int> actions;
	AbstractNode* exit_next_node;

	Network* original_network;
	Network* branch_network;

	int state;
	int state_iter;

	int starting_iter;

	double existing_sum_scores;
	int existing_count;
	double new_sum_scores;
	int new_count;

	int curr_ramp;
	/**
	 * - simply init to 0 and fast fail
	 */
	int measure_status;

	double local_improvement;
	double global_improvement;

	Experiment();
	~Experiment();

	void experiment_activate(ExperimentRun* run);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 ExperimentRun* run);

	void predict_activate(PredictRun* run);
	void predict_step(PredictRun* run);

	void backprop(double target_val,
				  ExperimentHistory* history,
				  Wrapper* wrapper);

	void add(Wrapper* wrapper);
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