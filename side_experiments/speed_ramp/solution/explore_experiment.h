#ifndef EXPLORE_EXPERIMENT_H
#define EXPLORE_EXPERIMENT_H

#include <list>
#include <map>
#include <vector>

#include "abstract_experiment.h"
#include "input.h"

class AbstractNode;
class Network;
class Scope;
class ScopeHistory;
class SolutionWrapper;

const int EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
/**
 * - simply train existing before explore
 */
const int EXPLORE_EXPERIMENT_STATE_EXPLORE = 1;
const int EXPLORE_EXPERIMENT_STATE_TRAIN_NEW = 2;
const int EXPLORE_EXPERIMENT_STATE_MEASURE = 3;

class ExploreExperimentHistory;
class ExploreExperimentState;
class ExploreExperiment : public AbstractExperiment {
public:
	AbstractNode* node_context;
	bool is_branch;
	AbstractNode* exit_next_node;

	std::list<int> last_num_following_explores;
	int sum_num_following_explores;
	std::list<int> last_num_instances;
	int sum_num_instances;

	int state;
	int state_iter;

	double existing_constant;
	std::vector<Input> existing_inputs;
	std::vector<double> existing_input_averages;
	std::vector<double> existing_input_standard_deviations;
	std::vector<double> existing_weights;
	std::vector<Input> existing_network_inputs;
	Network* existing_network;
	/**
	 * - simply don't bother reusing existing_network
	 */

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

	double select_percentage;

	double new_constant;
	std::vector<Input> new_inputs;
	std::vector<double> new_input_averages;
	std::vector<double> new_input_standard_deviations;
	std::vector<double> new_weights;
	std::vector<Input> new_network_inputs;
	Network* new_network;

	std::vector<ScopeHistory*> scope_histories;
	std::vector<double> target_val_histories;

	double existing_sum_scores;
	int existing_count;
	double new_sum_scores;
	int new_count;

	ExploreExperiment(AbstractNode* node_context,
					  bool is_branch,
					  AbstractNode* exit_next_node);
	~ExploreExperiment();

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
				  ExploreExperimentHistory* history,
				  SolutionWrapper* wrapper);

	void train_existing_check_activate(SolutionWrapper* wrapper,
									   ExploreExperimentHistory* history);
	void train_existing_backprop(double target_val,
								 ExploreExperimentHistory* history,
								 SolutionWrapper* wrapper);

	void explore_check_activate(SolutionWrapper* wrapper,
								ExploreExperimentHistory* history);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper,
					  ExploreExperimentState* experiment_state);
	void explore_set_action(int action,
							ExploreExperimentState* experiment_state);
	void explore_exit_step(SolutionWrapper* wrapper,
						   ExploreExperimentState* experiment_state);
	void explore_backprop(double target_val,
						  ExploreExperimentHistory* history,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(SolutionWrapper* wrapper,
								  ExploreExperimentHistory* history);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper,
						ExploreExperimentState* experiment_state);
	void train_new_exit_step(SolutionWrapper* wrapper,
							 ExploreExperimentState* experiment_state);
	void train_new_backprop(double target_val,
							ExploreExperimentHistory* history,
							SolutionWrapper* wrapper);

	void measure_check_activate(SolutionWrapper* wrapper,
								  ExploreExperimentHistory* history);
	void measure_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper,
					  ExploreExperimentState* experiment_state);
	void measure_exit_step(SolutionWrapper* wrapper,
						   ExploreExperimentState* experiment_state);
	void measure_backprop(double target_val,
						  ExploreExperimentHistory* history,
						  SolutionWrapper* wrapper);
};

class ExploreExperimentHistory {
public:
	bool is_on;

	int num_instances;

	std::vector<double> sum_signal_vals;
	std::vector<int> sum_counts;
	std::vector<double> existing_predicted_scores;

	ExploreExperimentHistory(ExploreExperiment* experiment,
							 SolutionWrapper* wrapper);
};

class ExploreExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExploreExperimentState(ExploreExperiment* experiment);
};

bool train_helper(std::vector<ScopeHistory*>& scope_histories,
				  std::vector<double>& target_val_histories,
				  double& constant,
				  std::vector<Input>& factor_inputs,
				  std::vector<double>& factor_input_averages,
				  std::vector<double>& factor_input_standard_deviations,
				  std::vector<double>& factor_weights,
				  std::vector<Input>& network_inputs,
				  Network*& network,
				  double& select_percentage);

#endif /* EXPLORE_EXPERIMENT_H */