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

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SEQUENCE = 1;

const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 1;
const int BRANCH_EXPERIMENT_STATE_TRAIN_PRE = 2;
const int BRANCH_EXPERIMENT_STATE_TRAIN = 3;
const int BRANCH_EXPERIMENT_STATE_TRAIN_POST = 4;
const int BRANCH_EXPERIMENT_STATE_MEASURE_COMBINED = 5;
const int BRANCH_EXPERIMENT_STATE_MEASURE_PASS_THROUGH = 6;

const int BRANCH_EXPERIMENT_STATE_FAIL = 7;
const int BRANCH_EXPERIMENT_STATE_SUCCESS = 8;

class BranchExperimentHistory;
class BranchExperiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

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

	int containing_scope_num_input_states;
	int containing_scope_num_local_states;

	double existing_average_score;
	double existing_average_misguess;
	Eigen::MatrixXd* existing_starting_state_vals;
	std::vector<double> existing_target_vals;
	std::vector<double> existing_starting_input_state_weights;
	std::vector<double> existing_starting_local_state_weights;

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

	bool recursion_protection;
	bool need_recursion_protection;

	std::list<ScopeHistory*> new_starting_scope_histories;
	Eigen::MatrixXd* new_starting_state_vals;
	std::list<ScopeHistory*> new_ending_scope_histories;
	std::vector<double> new_target_val_histories;

	double new_average_score;

	std::vector<double> new_starting_input_state_weights;
	std::vector<double> new_starting_local_state_weights;
	std::vector<double> new_starting_experiment_state_weights;

	std::vector<double> new_ending_input_state_weights;
	std::vector<double> new_ending_local_state_weights;

	double new_average_misguess;
	double new_misguess_variance;

	std::vector<State*> new_states;
	std::vector<std::vector<AbstractNode*>> new_state_nodes;
	std::vector<std::vector<std::vector<int>>> new_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> new_state_node_contexts;
	std::vector<std::vector<int>> new_state_obs_indexes;
	std::vector<double> new_state_weights;

	double combined_score;
	int branch_count;
	int branch_possible;

	double pass_through_misguess;

	int pass_through_selected_count;
	double pass_through_score;

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

	std::vector<ActionNodeHistory*> action_histories;
	std::vector<SequenceHistory*> sequence_histories;

	double existing_predicted_score;

	ScopeHistory* parent_scope_history;

	BranchExperimentHistory(BranchExperiment* experiment);
	BranchExperimentHistory(BranchExperimentHistory* original);
	~BranchExperimentHistory();
};

#endif /* BRANCH_EXPERIMENT_H */