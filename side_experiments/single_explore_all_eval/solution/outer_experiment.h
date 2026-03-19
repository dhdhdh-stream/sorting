#ifndef OUTER_EXPERIMENT_H
#define OUTER_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class SolutionWrapper;

const int OUTER_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int OUTER_EXPERIMENT_STATE_EXPLORE = 1;
const int OUTER_EXPERIMENT_STATE_TRAIN_NEW = 2;
const int OUTER_EXPERIMENT_STATE_REMEASURE_EXISTING = 3;
const int OUTER_EXPERIMENT_STATE_MEASURE = 4;

#if defined(MDEBUG) && MDEBUG
const int OUTER_MEASURE_STEP_NUM_ITERS = 10;
#else
const int OUTER_MEASURE_STEP_NUM_ITERS = 2000;
#endif /* MDEBUG */

class OuterExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	std::vector<std::vector<double>> existing_obs_histories;
	std::vector<double> existing_true_histories;

	Network* existing_true_network;

	int sum_num_instances;

	double average_instances_per_run;
	int num_instances_until_target;

	Scope* curr_new_scope;
	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;

	double best_surprise;
	Scope* best_new_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;

	std::vector<std::vector<double>> new_obs_histories;
	std::vector<double> new_true_histories;

	std::vector<Network*> new_networks;

	std::vector<double> existing_scores;
	std::vector<double> new_scores;

	double total_sum_scores;
	int total_count;

	double local_improvement;
	double global_improvement;
	double score_standard_deviation;

	OuterExperiment(Scope* scope_context,
					AbstractNode* node_context,
					bool is_branch,
					AbstractNode* exit_next_node);
	~OuterExperiment();

	void experiment_check_activate(AbstractNode* experiment_node,
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

	void remeasure_existing_check_activate(SolutionWrapper* wrapper);
	void remeasure_existing_step(std::vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper);
	void remeasure_existing_backprop(double target_val,
									 SolutionWrapper* wrapper);

	void measure_check_activate(SolutionWrapper* wrapper);
	void measure_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper);
	void measure_exit_step(SolutionWrapper* wrapper);
	void measure_backprop(double target_val,
						  SolutionWrapper* wrapper);

	void add(SolutionWrapper* wrapper);
};

class OuterExperimentHistory {
public:
	OuterExperiment* experiment;

	bool is_hit;

	std::vector<double> existing_predicted_trues;

	bool hit_branch;

	OuterExperimentHistory(OuterExperiment* experiment);
};

class OuterExperimentState : public AbstractExperimentState {
public:
	int step_index;

	OuterExperimentState(OuterExperiment* experiment);
};

#endif /* OUTER_EXPERIMENT_H */