#ifndef BRANCH_COMPARE_EXPERIMENT_H
#define BRANCH_COMPARE_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "input.h"

class BranchNode;
class Network;
class Scope;
class SolutionWrapper;

const int BRANCH_COMPARE_EXPERIMENT_STATE_EXPLORE = 0;
const int BRANCH_COMPARE_EXPERIMENT_STATE_TRAIN_NEW = 1;
const int BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_TRUE = 2;
const int BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_SIGNAL = 3;

class BranchCompareExperimentHistory;
class BranchCompareExperimentState;
class BranchCompareExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	double existing_signal_average_score;
	std::vector<Input> existing_signal_inputs;
	std::vector<double> existing_signal_input_averages;
	std::vector<double> existing_signal_input_standard_deviations;
	std::vector<double> existing_signal_weights;

	double average_instances_per_run;
	int num_instances_until_target;

	double curr_surprise;
	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	double new_true_average_score;
	std::vector<Input> new_true_inputs;
	std::vector<double> new_true_input_averages;
	std::vector<double> new_true_input_standard_deviations;
	std::vector<double> new_true_weights;
	std::vector<Input> new_true_network_inputs;
	Network* new_true_network;

	double new_signal_average_score;
	std::vector<Input> new_signal_inputs;
	std::vector<double> new_signal_input_averages;
	std::vector<double> new_signal_input_standard_deviations;
	std::vector<double> new_signal_weights;
	std::vector<Input> new_signal_network_inputs;
	Network* new_signal_network;

	BranchNode* new_branch_node;
	std::vector<AbstractNode*> new_nodes;

	std::vector<ScopeHistory*> scope_histories;
	std::vector<double> true_target_val_histories;
	std::vector<double> signal_target_val_histories;

	BranchCompareExperiment(Scope* scope_context,
							AbstractNode* node_context,
							bool is_branch);
	~BranchCompareExperiment();
	void decrement(AbstractNode* experiment_node);

	void check_activate(AbstractNode* experiment_node,
						bool is_branch,
						SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 bool& fetch_action,
						 SolutionWrapper* wrapper);
	void set_action(int action,
					SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);
	void back_activate(SolutionWrapper* wrapper);
	void backprop(double target_val,
				  SolutionWrapper* wrapper);

	void explore_check_activate(SolutionWrapper* wrapper,
								BranchCompareExperimentHistory* history);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper,
					  BranchCompareExperimentState* experiment_state);
	void explore_set_action(int action,
							BranchCompareExperimentState* experiment_state);
	void explore_exit_step(SolutionWrapper* wrapper,
						   BranchCompareExperimentState* experiment_state);
	void explore_back_activate(SolutionWrapper* wrapper);
	void explore_backprop(double target_val,
						  BranchCompareExperimentHistory* history);

	void train_new_check_activate(SolutionWrapper* wrapper,
								  BranchCompareExperimentHistory* history);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper,
						BranchCompareExperimentState* experiment_state);
	void train_new_exit_step(SolutionWrapper* wrapper,
							 BranchCompareExperimentState* experiment_state);
	void train_new_back_activate(SolutionWrapper* wrapper);
	void train_new_backprop(double target_val,
							BranchCompareExperimentHistory* history);

	void measure_true_check_activate(SolutionWrapper* wrapper);
	void measure_true_step(std::vector<double>& obs,
						   int& action,
						   bool& is_next,
						   SolutionWrapper* wrapper,
						   BranchCompareExperimentState* experiment_state);
	void measure_true_exit_step(SolutionWrapper* wrapper,
								BranchCompareExperimentState* experiment_state);
	void measure_true_backprop(double target_val,
							   BranchCompareExperimentHistory* history);

	void measure_signal_check_activate(SolutionWrapper* wrapper);
	void measure_signal_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper,
							 BranchCompareExperimentState* experiment_state);
	void measure_signal_exit_step(SolutionWrapper* wrapper,
								  BranchCompareExperimentState* experiment_state);
	void measure_signal_backprop(double target_val,
								 BranchCompareExperimentHistory* history);

	void clean();
	void add(SolutionWrapper* wrapper);
};

class BranchCompareExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<double> existing_predicted_scores;

	BranchCompareExperimentHistory(BranchCompareExperiment* experiment);
};

class BranchCompareExperimentState : public AbstractExperimentState {
public:
	int step_index;

	BranchCompareExperimentState(BranchCompareExperiment* experiment);
};

#endif /* BRANCH_COMPARE_EXPERIMENT_H */