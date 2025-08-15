#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <map>
#include <vector>

#include "abstract_experiment.h"
#include "input.h"

class BranchNode;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class SolutionWrapper;

const int BRANCH_EXPERIMENT_STATE_EXPLORE = 0;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 1;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 2;
#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 3;
#endif /* MDEBUG */

const int EXPERIMENT_RESULT_NA = 0;
const int EXPERIMENT_RESULT_FAIL = 1;
const int EXPERIMENT_RESULT_SUCCESS = 2;

class BranchExperimentState;
class BranchExperiment : public AbstractExperiment {
public:
	Scope* scope_context;
	AbstractNode* node_context;
	bool is_branch;

	int state;
	int state_iter;

	int result;
	double improvement;

	double existing_average_score;
	std::vector<Input> existing_inputs;
	std::vector<double> existing_input_averages;
	std::vector<double> existing_input_standard_deviations;
	std::vector<double> existing_weights;

	double average_instances_per_run;
	int num_instances_until_target;

	Scope* curr_new_scope;
	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;
	ScopeHistory* curr_scope_history;

	double best_surprise;
	Scope* best_new_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;
	ScopeHistory* best_scope_history;

	double new_average_score;
	std::vector<Input> new_inputs;
	std::vector<double> new_input_averages;
	std::vector<double> new_input_standard_deviations;
	std::vector<double> new_weights;
	std::vector<Input> new_network_inputs;
	Network* new_network;

	double select_percentage;

	BranchNode* new_branch_node;
	std::vector<AbstractNode*> new_nodes;

	std::vector<ScopeHistory*> scope_histories;
	std::vector<bool> match_histories;
	std::vector<double> i_target_val_histories;

	std::vector<double> existing_scores;
	std::vector<double> new_scores;

	std::vector<ScopeHistory*> explore_scope_histories;
	std::vector<AbstractNode*> explore_node_context;

	std::vector<ScopeHistory*> new_scope_histories;
	std::vector<double> new_target_val_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	BranchExperiment(Scope* scope_context,
					 AbstractNode* node_context,
					 bool is_branch,
					 SolutionWrapper* wrapper);
	~BranchExperiment();
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
	void backprop(double target_val,
				  SolutionWrapper* wrapper);

	void explore_check_activate(SolutionWrapper* wrapper);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper,
					  BranchExperimentState* experiment_state);
	void explore_set_action(int action,
							BranchExperimentState* experiment_state);
	void explore_exit_step(SolutionWrapper* wrapper,
						   BranchExperimentState* experiment_state);
	void explore_backprop(double target_val,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(SolutionWrapper* wrapper);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper,
						BranchExperimentState* experiment_state);
	void train_new_exit_step(SolutionWrapper* wrapper,
							 BranchExperimentState* experiment_state);
	void train_new_backprop(double target_val,
							SolutionWrapper* wrapper);

	void measure_check_activate(SolutionWrapper* wrapper);
	void measure_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper,
					  BranchExperimentState* experiment_state);
	void measure_exit_step(SolutionWrapper* wrapper,
						   BranchExperimentState* experiment_state);
	void measure_backprop(double target_val,
						  SolutionWrapper* wrapper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_check_activate(SolutionWrapper* wrapper);
	void capture_verify_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper,
							 BranchExperimentState* experiment_state);
	void capture_verify_exit_step(SolutionWrapper* wrapper,
								  BranchExperimentState* experiment_state);
	void capture_verify_backprop(SolutionWrapper* wrapper);
	#endif /* MDEBUG */

	void clean();
	void add(SolutionWrapper* wrapper);
};

class BranchExperimentOverallHistory {
public:
	BranchExperiment* experiment;

	bool is_hit;

	BranchExperimentOverallHistory(BranchExperiment* experiment);
};

class BranchExperimentInstanceHistory {
public:
	BranchExperiment* experiment;

	double existing_predicted_score;

	ScopeHistory* signal_needed_from;

	BranchExperimentInstanceHistory(BranchExperiment* experiment);
};

class BranchExperimentState : public AbstractExperimentState {
public:
	int step_index;

	BranchExperimentState(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */