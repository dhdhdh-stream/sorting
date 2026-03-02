#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class SolutionWrapper;

const int EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int EXPERIMENT_STATE_CLEAN = 1;
const int EXPERIMENT_STATE_EXPLORE = 2;
const int EXPERIMENT_STATE_TRAIN_NEW = 3;
const int EXPERIMENT_STATE_REFINE = 4;
const int EXPERIMENT_STATE_MEASURE = 5;
#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_STATE_CAPTURE_VERIFY = 6;
#endif /* MDEBUG */

class Experiment : public AbstractExperiment {
public:
	bool can_clean;

	int state;
	int state_iter;

	std::vector<std::vector<double>> existing_obs_histories;
	std::vector<double> existing_true_histories;

	double existing_true;

	Network* existing_true_network;

	int sum_num_instances;

	double average_instances_per_run;
	int num_instances_until_target;

	bool clean_success;

	Scope* curr_new_scope;
	Scope* curr_parent_scope;
	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;
	std::vector<AbstractNode*> curr_new_nodes;

	double best_surprise;
	Scope* best_new_scope;
	Scope* best_parent_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;
	std::vector<AbstractNode*> best_new_nodes;

	std::vector<std::vector<double>> new_obs_histories;
	std::vector<double> new_true_histories;

	Network* new_true_network;
	Network* refine_network;

	bool best_new_is_binarize;
	bool best_refine_is_binarize;

	double sum_true;
	int hit_count;

	double total_sum_scores;
	int total_count;

	double global_improvement;
	double local_improvement;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<std::vector<double>> verify_scores;
	#endif /* MDEBUG */

	Experiment(Scope* scope_context,
			   AbstractNode* node_context,
			   bool is_branch,
			   AbstractNode* exit_next_node,
			   bool can_clean);
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

	void clean_check_activate(SolutionWrapper* wrapper);
	void clean_backprop(double target_val,
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

	void refine_check_activate(SolutionWrapper* wrapper);
	void refine_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper);
	void refine_exit_step(SolutionWrapper* wrapper);
	void refine_backprop(double target_val,
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
	double calc_new_score();

	void train_and_eval_helper(double& best_improvement,
							   Network*& best_network,
							   bool& best_is_binarize);

	void clean_add_helper(SolutionWrapper* wrapper);
	void experiment_add_helper(SolutionWrapper* wrapper);
};

class ExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<double> existing_predicted_trues;

	ExperimentHistory(Experiment* experiment);
};

class ExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExperimentState(Experiment* experiment);
};

#endif /* EXPERIMENT_H */