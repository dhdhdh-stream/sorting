#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <list>
#include <map>
#include <vector>
#include <Eigen/Dense>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"
#include "state_status.h"

class AbstractNode;
class ActionNode;
class ActionNodeHistory;
class ObsExperiment;
class Scale;
class ScopeHistory;
class Sequence;
class SequenceHistory;
class State;

const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 1;
const int BRANCH_EXPERIMENT_STATE_TRAIN = 2;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 3;

const int BRANCH_EXPERIMENT_STATE_FAIL = 4;
const int BRANCH_EXPERIMENT_STATE_SUCCESS = 5;

const int EXPERIMENT_SURPRISE_THRESHOLD = 1.0;
/**
 * - if surprise isn't better than what can be expected from random fluctuation, don't bother
 */

class BranchExperimentHistory;
class BranchExperiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

	PassThroughExperiment* parent_pass_through_experiment;

	double average_remaining_experiments_from_start;
	double average_instances_per_run;
	/**
	 * - when triggering an experiment, it becomes live everywhere
	 *   - for one selected instance, always trigger branch and experiment
	 *     - for everywhere else, trigger accordingly
	 * 
	 * - set probabilities after average_instances_per_run to 50%
	 */

	int state;
	int state_iter;

	double existing_average_score;
	// TODO: add
	double existing_average_misguess;

	std::vector<std::vector<std::map<int, StateStatus>>> input_state_vals_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> local_state_vals_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> score_state_vals_histories;
	std::vector<std::map<int, StateStatus>> experiment_state_vals_histories;
	std::vector<double> target_val_histories;

	std::vector<vector<double>> existing_input_state_weights;
	std::vector<vector<double>> existing_local_state_weights;
	std::vector<vector<double>> existing_score_state_weights;
	std::vector<double> existing_experiment_state_weights;

	int existing_selected_count;
	double existing_selected_sum_score;

	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<Sequence*> curr_sequences;
	int curr_exit_depth;
	int curr_exit_node_id;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<Sequence*> best_sequences;
	int best_exit_depth;
	int best_exit_node_id;

	std::vector<std::vector<double>> new_input_state_weights;
	std::vector<std::vector<double>> new_local_state_weights;
	std::vector<std::vector<double>> new_score_state_weights;
	std::vector<double> new_experiment_state_weights;

	std::vector<State*> new_states;
	std::vector<std::vector<AbstractNode*>> new_state_nodes;
	std::vector<std::vector<std::vector<int>>> new_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> new_state_node_contexts;
	std::vector<std::vector<int>> new_state_obs_indexes;

	double combined_score;
	int branch_count;
	int branch_possible;

	std::vector<std::vector<int>> obs_experiment_obs_indexes;
	std::vector<std::vector<double>> obs_experiment_obs_vals;
	ObsExperiment* obs_experiment;

	BranchExperiment(std::vector<int> scope_context,
					 std::vector<int> node_context);
	~BranchExperiment();

	void activate(int& curr_node_id,
				  Problem& problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  int& exit_node_id,
				  RunHelper& run_helper,
				  BranchExperimentHistory*& history);
	void hook(std::vector<ContextLayer>& context);
	void hook_helper(std::vector<int>& scope_context,
					 std::vector<int>& node_context,
					 std::map<int, StateStatus>& experiment_state_vals,
					 ScopeHistory* scope_history);
	void unhook();
	void backprop(double target_val,
				  BranchExperimentHistory* history);

	void train_existing_activate(std::vector<ContextLayer>& context);
	void train_existing_backprop(double target_val,
								 BranchExperimentHistory* history);

	void explore_activate(int& curr_node_id,
						  Problem& problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  int& exit_node_id,
						  RunHelper& run_helper,
						  BranchExperimentHistory* history);
	void explore_backprop(double target_val,
						  BranchExperimentHistory* history);

	void train_activate(int& curr_node_id,
						Problem& problem,
						std::vector<ContextLayer>& context,
						int& exit_depth,
						int& exit_node_id,
						RunHelper& run_helper,
						BranchExperimentHistory* history);
	void train_backprop(double target_val,
						BranchExperimentHistory* history);
	void process_train();

	void simple_activate(int& curr_node_id,
						 Problem& problem,
						 std::vector<ContextLayer>& context,
						 int& exit_depth,
						 int& exit_node_id,
						 RunHelper& run_helper,
						 BranchExperimentHistory*& history);
	void simple_combined_activate(int& curr_node_id,
								  Problem& problem,
								  std::vector<ContextLayer>& context,
								  int& exit_depth,
								  int& exit_node_id,
								  RunHelper& run_helper);
	void simple_pass_through_activate(int& curr_node_id,
									  Problem& problem,
									  std::vector<ContextLayer>& context,
									  int& exit_depth,
									  int& exit_node_id,
									  RunHelper& run_helper);

	void measure_pass_through_activate(int& curr_node_id,
									   Problem& problem,
									   std::vector<ContextLayer>& context,
									   int& exit_depth,
									   int& exit_node_id,
									   RunHelper& run_helper);
	void measure_pass_through_backprop(double target_val,
									   BranchExperimentHistory* history);

	void eval();
	void new_branch();
	void new_pass_through();
};

class BranchExperimentHistory {
public:
	BranchExperiment* experiment;

	double existing_predicted_score;

	std::vector<ActionNodeHistory*> action_histories;
	std::vector<SequenceHistory*> sequence_histories;

	BranchExperimentHistory(BranchExperiment* experiment);
	BranchExperimentHistory(BranchExperimentHistory* original);
	~BranchExperimentHistory();
};

#endif /* BRANCH_EXPERIMENT_H */