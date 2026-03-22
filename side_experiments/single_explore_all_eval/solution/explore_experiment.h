#ifndef EXPLORE_EXPERIMENT_H
#define EXPLORE_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class SolutionWrapper;

const int EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int EXPLORE_EXPERIMENT_STATE_EXPLORE = 1;
const int EXPLORE_EXPERIMENT_STATE_TRAIN_NEW = 2;

class ExploreExperiment : public AbstractExperiment {
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

	int total_count;

	std::vector<Network*> new_networks;

	ExploreExperiment(Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractNode* exit_next_node);
	~ExploreExperiment();

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
};

class ExploreExperimentHistory {
public:
	ExploreExperiment* experiment;

	bool is_hit;

	std::vector<double> existing_predicted_trues;

	ExploreExperimentHistory(ExploreExperiment* experiment);
};

class ExploreExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExploreExperimentState(ExploreExperiment* experiment);
};

#endif /* EXPLORE_EXPERIMENT_H */