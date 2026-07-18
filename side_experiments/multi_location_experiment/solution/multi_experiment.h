#ifndef MULTI_EXPERIMENT_H
#define MULTI_EXPERIMENT_H

#include <set>
#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class SolutionWrapper;

const int MULTI_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int MULTI_EXPERIMENT_STATE_EXPLORE = 1;
const int MULTI_EXPERIMENT_STATE_TRAIN_NEW = 2;

class MultiExperimentHistory;
class MultiExperiment : public AbstractExperiment {
public:
	Scope* scope_context;
	std::vector<AbstractNode*> node_contexts;
	std::vector<bool> is_branch;

	int state;
	int state_iter;

	std::vector<std::vector<double>> existing_obs_histories;
	std::vector<double> existing_target_val_histories;

	Network* existing_network;

	double average_instances_per_hit;
	int num_instances_until_target;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;

	int start_iter;

	std::vector<std::vector<double>> new_obs_histories;
	std::vector<double> new_target_val_histories;

	Network* new_network;

	MultiExperiment(Scope* scope_context,
					std::vector<AbstractNode*>& node_contexts,
					std::vector<bool>& is_branch,
					SolutionWrapper* wrapper);
	~MultiExperiment();

	void experiment_check_activate(std::vector<double>& obs,
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
				  AbstractExperimentHistory* history,
				  SolutionWrapper* wrapper);

	void train_existing_check_activate(std::vector<double>& obs,
									   MultiExperimentHistory* history,
									   SolutionWrapper* wrapper);
	void train_existing_backprop(double target_val,
								 MultiExperimentHistory* history,
								 SolutionWrapper* wrapper);

	void explore_check_activate(std::vector<double>& obs,
								MultiExperimentHistory* history,
								SolutionWrapper* wrapper);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper);
	void explore_set_action(int action,
							SolutionWrapper* wrapper);
	void explore_exit_step(SolutionWrapper* wrapper);
	void explore_backprop(double target_val,
						  MultiExperimentHistory* history,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(std::vector<double>& obs,
								  MultiExperimentHistory* history,
								  SolutionWrapper* wrapper);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper);
	void train_new_exit_step(SolutionWrapper* wrapper);
	void train_new_backprop(double target_val,
							MultiExperimentHistory* history,
							SolutionWrapper* wrapper);

	void add(SolutionWrapper* wrapper);
};

class MultiExperimentHistory : public AbstractExperimentHistory {
public:
	int num_instances;

	std::vector<std::vector<double>> obs_histories;

	std::vector<double> existing_predicted;

	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;

	MultiExperimentHistory(MultiExperiment* experiment);
};

class MultiExperimentState : public AbstractExperimentState {
public:
	int step_index;

	MultiExperimentState(MultiExperiment* experiment);
};

#endif /* EXPLORE_EXPERIMENT_H */