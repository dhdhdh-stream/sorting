#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class BranchNode;
class Network;
class SolutionWrapper;

const int EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int EXPERIMENT_STATE_EXPLORE = 1;
const int EXPERIMENT_STATE_TRAIN_NEW = 2;
const int EXPERIMENT_STATE_MEASURE = 3;
#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_STATE_CAPTURE_VERIFY = 4;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_EXPLORE_ITERS = 10;
#else
const int EXPERIMENT_EXPLORE_ITERS = 200;
#endif /* MDEBUG */

class Experiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	std::vector<std::vector<double>> existing_obs_histories;
	std::vector<std::vector<double>> existing_target_vals;
	std::vector<std::vector<bool>> existing_target_vals_is_on;

	// temp
	std::vector<std::vector<double>> existing_predicted;

	double existing_true;

	// std::vector<Network*> existing_networks;
	// std::vector<double> existing_misguess_standard_deviations;

	int sum_num_instances;

	double average_instances_per_run;
	int num_instances_until_target;

	std::vector<double> curr_explore_obs_history;
	Scope* curr_new_scope;
	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;
	std::vector<AbstractNode*> curr_new_nodes;

	double best_surprise;
	Scope* best_new_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;
	std::vector<AbstractNode*> best_new_nodes;

	std::vector<std::vector<double>> new_obs_histories;
	std::vector<std::vector<double>> new_target_vals;
	std::vector<std::vector<bool>> new_target_vals_is_on;

	Network* new_network;

	/**
	 * - for debugging
	 */
	int best_new_layer;
	bool best_new_is_binarize;

	BranchNode* new_branch_node;

	double sum_true;
	int hit_count;

	std::vector<double> total_scores;

	double improvement;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	Experiment(Scope* scope_context,
			   AbstractNode* node_context,
			   bool is_branch);
	~Experiment();

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

	void train_existing_check_activate(SolutionWrapper* wrapper);
	void train_existing_step(std::vector<double>& obs,
							 SolutionWrapper* wrapper);
	void train_existing_backprop(double target_val,
								 SolutionWrapper* wrapper);

	void explore_check_activate(SolutionWrapper* wrapper);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper);
	void explore_set_action(int action,
							SolutionWrapper* wrapper);
	void explore_exit_step(SolutionWrapper* wrapper);
	void explore_backprop(double target_val,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(SolutionWrapper* wrapper);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper);
	void train_new_exit_step(SolutionWrapper* wrapper);
	void train_new_backprop(double target_val,
							SolutionWrapper* wrapper);

	void measure_check_activate(SolutionWrapper* wrapper);
	void measure_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper);
	void measure_exit_step(SolutionWrapper* wrapper);
	void measure_backprop(double target_val,
						  SolutionWrapper* wrapper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_check_activate(SolutionWrapper* wrapper);
	void capture_verify_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper);
	void capture_verify_exit_step(SolutionWrapper* wrapper);
	void capture_verify_backprop(SolutionWrapper* wrapper);
	#endif /* MDEBUG */

	void clean();
	void add(SolutionWrapper* wrapper);
	void calc_new_score(double& new_average,
						double& new_standard_deviation);

	void train_and_eval_helper(int layer,
							   double& best_improvement,
							   Network*& best_network,
							   int& best_layer,
							   bool& best_is_binarize);
};

class ExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<std::vector<ScopeHistory*>> stack_traces;
	std::vector<std::vector<int>> explore_indexes;

	ExperimentHistory(Experiment* experiment);
};

class ExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExperimentState(Experiment* experiment);
};

#endif /* EXPERIMENT_H */