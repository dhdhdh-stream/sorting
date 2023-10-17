#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <map>
#include <vector>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"
#include "state_status.h"

class AbstractNode;
class ActionNode;
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
const int BRANCH_EXPERIMENT_STATE_MEASURE_EXISTING = 5;
const int BRANCH_EXPERIMENT_STATE_MEASURE_NEW = 6;
const int BRANCH_EXPERIMENT_STATE_MEASURE_PASS_THROUGH = 7;
const int BRANCH_EXPERIMENT_STATE_DONE = 8;

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
	 *       - so experiment score_state scales need to be adjusted after ObsExperiment
	 * 
	 * - set probabilities after average_instances_per_run to 50%
	 */

	int state;
	int state_iter;

	double existing_average_score;
	double existing_average_misguess;
	std::map<int, Scale*> existing_starting_input_state_scales;
	std::map<int, Scale*> existing_starting_local_state_scales;
	std::map<State*, Scale*> existing_starting_score_state_scales;

	int new_scope_id;
	/**
	 * - always create as extra scope
	 *   - good to create scopes in general for abstraction
	 *   - but also may need extra scopes to capture correlated information, e.g.:
	 *     - on outer, capture early to branch early
	 *     - on inner, capture later so can be abstracted
	 */

	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<Sequence*> curr_sequences;
	int curr_exit_depth;
	int curr_exit_node_id;

	double best_surprise;
	/**
	 * - score improvement divided by average_misguess
	 */
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<Sequence*> best_sequences;
	int best_exit_depth;
	int best_exit_node_id;

	double new_average_score;
	double new_average_misguess;
	double new_misguess_variance;

	std::map<int, Scale*> new_starting_input_state_scales;
	std::map<int, Scale*> new_starting_local_state_scales;
	std::map<State*, Scale*> new_starting_score_state_scales;
	std::map<State*, Scale*> new_starting_experiment_score_state_scales;

	std::map<int, Scale*> new_ending_input_state_scales;
	std::map<int, Scale*> new_ending_local_state_scales;
	std::map<State*, Scale*> new_ending_score_state_scales;

	std::vector<State*> new_score_states;
	std::vector<std::vector<AbstractNode*>> new_score_state_nodes;
	std::vector<std::vector<std::vector<int>>> new_score_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> new_score_state_node_contexts;
	std::vector<std::vector<int>> new_score_state_obs_indexes;
	std::map<State*, std::pair<Scale*, double>> new_score_state_scales;

	double branch_existing_score;
	int existing_branch_count;
	double non_branch_existing_score;

	double branch_new_score;
	int new_branch_count;
	double non_branch_new_score;

	double pass_through_score;

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
	void hook(std::vector<ContextLayer>& context,
			  RunHelper& run_helper);
	void hook_helper(std::vector<int>& scope_context,
					 std::vector<int>& node_context,
					 std::map<State*, StateStatus>& experiment_score_state_vals,
					 std::vector<int>& test_obs_indexes,
					 std::vector<double>& test_obs_vals,
					 RunHelper& run_helper,
					 ScopeHistory* scope_history);
	void unhook();
	void backprop(double target_val,
				  BranchExperimentHistory* history);

	void train_existing_activate(std::vector<ContextLayer>& context,
								 BranchExperimentHistory* history);
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

	void simple_activate(int& curr_node_id,
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

	void measure_existing_activate(std::vector<ContextLayer>& context,
								   BranchExperimentHistory* history);
	void measure_existing_backprop(double target_val,
								   BranchExperimentHistory* history);

	void measure_new_activate(int& curr_node_id,
							  Problem& problem,
							  std::vector<ContextLayer>& context,
							  int& exit_depth,
							  int& exit_node_id,
							  RunHelper& run_helper,
							  BranchExperimentHistory* history);
	void measure_new_backprop(double target_val,
							  BranchExperimentHistory* history);

	void measure_pass_through_activate(int& curr_node_id,
									   Problem& problem,
									   std::vector<ContextLayer>& context,
									   int& exit_depth,
									   int& exit_node_id,
									   RunHelper& run_helper);
	void measure_pass_through_backprop(double target_val);

	void eval();
	void new_branch();
	void new_pass_through();
};

class BranchExperimentHistory {
public:
	BranchExperiment* experiment;

	std::map<int, StateStatus> starting_input_state_snapshots;
	std::map<int, StateStatus> starting_local_state_snapshots;
	std::map<State*, StateStatus> starting_score_state_snapshots;
	std::map<State*, StateStatus> starting_experiment_score_state_snapshots;

	std::vector<SequenceHistory*> sequence_histories;

	double existing_predicted_score;

	ScopeHistory* parent_scope_history;
	std::map<State*, StateStatus> ending_experiment_score_state_snapshots;

	bool is_branch;

	BranchExperimentHistory(BranchExperiment* experiment);
	~BranchExperimentHistory();
};

#endif /* BRANCH_EXPERIMENT_H */