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
								  AbstractNode* exit_next_node,
								  Wrapper* wrapper);

class ForceExperimentHistory;
class ForceExperiment : public AbstractExperiment {
public:
	AbstractNode* node_context;
	bool is_branch;
	AbstractNode* exit_next_node;

	int state;
	int state_iter;

	Network* original_network;

	double best_surprise;
	std::vector<int> best_actions;

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
	void backprop(double target_val,
				  ExperimentRun* run,
				  ForceExperimentHistory* history,
				  Wrapper* wrapper);

	void explore_experiment_activate(ExperimentRun* run);
	void explore_experiment_step(int& action,
								 bool& is_next,
								 ExperimentRun* run);
	void explore_backprop(double target_val,
						  ExperimentRun* run,
						  ForceExperimentHistory* history,
						  Wrapper* wrapper);

	void train_new_experiment_activate(ExperimentRun* run);
	void train_new_experiment_step(int& action,
								   bool& is_next,
								   ExperimentRun* run);
	void train_new_backprop(double target_val,
							ExperimentRun* run,
							ForceExperimentHistory* history,
							Wrapper* wrapper);

	bool further_than(ForceExperiment* other);
};

class ForceExperimentHistory {
public:
	ForceExperiment* experiment;

	double existing_predicted;

	std::vector<int> curr_actions;

	std::vector<std::vector<double>> branch_obs;
	std::vector<int> branch_actions;

	ForceExperimentHistory(ForceExperiment* experiment);
};

class ForceExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ForceExperimentState(ForceExperiment* experiment);
};

#endif /* FORCE_EXPERIMENT_H */