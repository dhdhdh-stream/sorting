/**
 * - too expensive to retrain networks
 *   - means having to activate/backprop every network
 *     - since affects error gradient
 */

#ifndef EXPLORE_EXPERIMENT_H
#define EXPLORE_EXPERIMENT_H

#include <set>
#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class ScoreNetwork;
class SolutionWrapper;

const int EXPLORE_EXPERIMENT_STATE_EXPLORE = 0;
const int EXPLORE_EXPERIMENT_STATE_TRAIN_NEW = 1;
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

	int num_instances_until_target;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;

	/**
	 * - -1 if scope start
	 */
	std::vector<std::vector<int>> dependencies;

	std::vector<std::vector<bool>> existing_dependencies_is_hit_histories;
	std::vector<std::vector<std::vector<double>>> existing_dependencies_state_histories;
	std::vector<std::vector<std::vector<double>>> existing_dependencies_obs_histories;
	std::vector<std::vector<double>> existing_state_histories;
	std::vector<double> existing_target_val_histories;

	std::vector<std::vector<bool>> new_dependencies_is_hit_histories;
	std::vector<std::vector<std::vector<double>>> new_dependencies_state_histories;
	std::vector<std::vector<std::vector<double>>> new_dependencies_obs_histories;
	std::vector<std::vector<double>> new_state_histories;
	std::vector<double> new_target_val_histories;

	ExploreExperiment(Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractNode* exit_next_node,
					  SolutionWrapper* wrapper);
	~ExploreExperiment();

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
				  ExploreExperimentHistory* history,
				  SolutionWrapper* wrapper);

	void explore_check_activate(std::vector<double>& obs,
								ExploreExperimentHistory* history,
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
						  ExploreExperimentHistory* history,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(std::vector<double>& obs,
								  ExploreExperimentHistory* history,
								  SolutionWrapper* wrapper);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper);
	void train_new_exit_step(SolutionWrapper* wrapper);
	void train_new_backprop(double target_val,
							ExploreExperimentHistory* history,
							SolutionWrapper* wrapper);

	void new_state_helper(SolutionWrapper* wrapper);

	void add(ScoreNetwork* new_network,
			 SolutionWrapper* wrapper);

	bool further_than(ExploreExperiment* other);
};

class ExploreExperimentHistory {
public:
	ExploreExperiment* experiment;

	std::vector<double> existing_predicted;

	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;

	std::vector<std::vector<bool>> dependencies_is_hit_histories;
	std::vector<std::vector<std::vector<double>>> dependencies_state_histories;
	std::vector<std::vector<std::vector<double>>> dependencies_obs_histories;
	std::vector<std::vector<double>> state_histories;

	ExploreExperimentHistory(ExploreExperiment* experiment);
};

class ExploreExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExploreExperimentState(ExploreExperiment* experiment);
};

#endif /* EXPLORE_EXPERIMENT_H */