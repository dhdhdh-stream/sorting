#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class BranchNode;
class Network;
class Scope;
class SolutionWrapper;

const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 1;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 2;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 3;
#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 4;
#endif /* MDEBUG */

class BranchExperimentHistory;
class BranchExperimentState;
class BranchExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	double existing_score;
	double existing_signal;

	Network* existing_network;

	int sum_instances;

	double average_instances_per_run;
	int num_instances_until_target;

	Scope* curr_new_scope;
	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	Scope* best_new_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	Network* new_network;

	int total_count;
	double total_sum_scores;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;

	double sum_scores;

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

	void check_activate(AbstractNode* experiment_node,
						bool is_branch,
						std::vector<double>& obs,
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

	void train_existing_check_activate(std::vector<double>& obs,
									   SolutionWrapper* wrapper,
									   BranchExperimentHistory* history);
	void train_existing_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper,
							 BranchExperimentState* experiment_state);
	void train_existing_backprop(double target_val,
								 SolutionWrapper* wrapper);

	void explore_check_activate(std::vector<double>& obs,
								SolutionWrapper* wrapper,
								BranchExperimentHistory* history);
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

	void train_new_check_activate(std::vector<double>& obs,
								  SolutionWrapper* wrapper,
								  BranchExperimentHistory* history);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper,
						BranchExperimentState* experiment_state);
	void train_new_exit_step(SolutionWrapper* wrapper,
							 BranchExperimentState* experiment_state);
	void train_new_backprop(double target_val,
							SolutionWrapper* wrapper);

	void measure_check_activate(std::vector<double>& obs,
								SolutionWrapper* wrapper,
								BranchExperimentHistory* history);
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
	void capture_verify_check_activate(std::vector<double>& obs,
									   SolutionWrapper* wrapper);
	void capture_verify_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper,
							 BranchExperimentState* experiment_state);
	void capture_verify_exit_step(SolutionWrapper* wrapper,
								  BranchExperimentState* experiment_state);
	void capture_verify_backprop(SolutionWrapper* wrapper);
	#endif /* MDEBUG */

	double calc_new_score();

	void clean();
	void add(SolutionWrapper* wrapper);
};

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<double> existing_predicted_scores;

	std::vector<double> signal_sum_vals;
	std::vector<int> signal_sum_counts;

	BranchExperimentHistory(BranchExperiment* experiment);
};

class BranchExperimentState : public AbstractExperimentState {
public:
	int step_index;

	BranchExperimentState(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */