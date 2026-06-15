#ifndef FORCE_EXPERIMENT_H
#define FORCE_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class ExperimentRun;
class Network;
class PredictRun;
class StateNetwork;
class Wrapper;

const int FORCE_EXPERIMENT_STATE_EXPLORE = 0;
const int FORCE_EXPERIMENT_STATE_TRAIN_NEW = 1;

const int FORCE_EXPERIMENT_NUM_NEW_STATE = 2;

void init_force_experiment_helper(AbstractNode* node_context,
								  bool is_branch,
								  Wrapper* wrapper);

class ForceExperimentHistory;
class ForceExperiment : public AbstractExperiment {
public:
	AbstractNode* node_context;
	bool is_branch;

	int state;
	int state_iter;

	Network* original_network;

	double curr_existing_predicted;
	std::vector<int> curr_actions;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	std::vector<int> best_actions;
	AbstractNode* best_exit_next_node;

	std::vector<double> existing_predicted;
	std::vector<std::vector<std::vector<double>>> new_branch_obs;
	std::vector<std::vector<int>> new_branch_actions;
	std::vector<std::vector<std::vector<double>>> new_full_obs;
	std::vector<std::vector<int>> new_full_actions;
	std::vector<double> new_target_vals;

	ForceExperiment();
	~ForceExperiment();

	void experiment_activate(ExperimentRun* run);
	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);
	void predict_activate(PredictRun* run);
	void backprop(double target_val,
				  ExperimentRun* run,
				  Wrapper* wrapper);

	void explore_experiment_activate(ExperimentRun* run);
	void explore_experiment_step(int& action,
								 bool& is_next,
								 ExperimentRun* run);
	void explore_backprop(double target_val,
						  ExperimentRun* run,
						  Wrapper* wrapper);

	void train_new_experiment_activate(ExperimentRun* run);
	void train_new_experiment_step(int& action,
								   bool& is_next,
								   ExperimentRun* run);
	void train_new_backprop(double target_val,
							ExperimentRun* run,
							Wrapper* wrapper);
};

class ForceExperimentHistory {
public:
	bool hit_branch;

	ForceExperimentHistory(ForceExperiment* experiment);
};

class ForceExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ForceExperimentState(ForceExperiment* experiment);
};

#endif /* FORCE_EXPERIMENT_H */