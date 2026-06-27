#ifndef EXPLORE_EXPERIMENT_H
#define EXPLORE_EXPERIMENT_H

#include <set>
#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class SolutionWrapper;

const int EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
/**
 * - simply always retrain existing_network
 *   - solution is always changing, so keeping an overall existing_network updated significant cost
 *   - also much easier to make mistakes
 *     - in case branches not properly taken into account
 */
const int EXPLORE_EXPERIMENT_STATE_EXPLORE = 1;
const int EXPLORE_EXPERIMENT_STATE_TRAIN_NEW = 2;
/**
 * - no measure step
 *   - improvement not just the experiment change itself...
 *     - ...but also the solution's adjustments to it
 *       - so any results from a measure step less meaningful
 */

class ExploreExperimentHistory;
class ExploreExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	std::vector<std::vector<double>> existing_obs_histories;
	std::vector<double> existing_target_val_histories;

	Network* existing_network;

	double average_instances_per_run;
	int num_instances_until_target;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;

	int start_iter;

	std::vector<std::vector<double>> new_obs_histories;
	std::vector<double> new_target_val_histories;

	Network* new_network;

	ExploreExperiment(Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractNode* exit_next_node,
					  SolutionWrapper* wrapper);
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
				  ExploreExperimentHistory* history,
				  SolutionWrapper* wrapper);

	void train_existing_check_activate(SolutionWrapper* wrapper);
	void train_existing_step(std::vector<double>& obs,
							 SolutionWrapper* wrapper);
	void train_existing_backprop(double target_val,
								 ExploreExperimentHistory* history,
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
						  ExploreExperimentHistory* history,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(SolutionWrapper* wrapper);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper);
	void train_new_exit_step(SolutionWrapper* wrapper);
	void train_new_backprop(double target_val,
							ExploreExperimentHistory* history,
							SolutionWrapper* wrapper);

	void add(SolutionWrapper* wrapper);

	bool further_than(ExploreExperiment* other);
};

class ExploreExperimentHistory {
public:
	ExploreExperiment* experiment;

	int num_instances;

	std::vector<std::vector<double>> obs_histories;

	std::vector<double> existing_predicted;

	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;

	ExploreExperimentHistory(ExploreExperiment* experiment);
};

class ExploreExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExploreExperimentState(ExploreExperiment* experiment);
};

#endif /* EXPLORE_EXPERIMENT_H */