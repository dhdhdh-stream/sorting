#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <map>
#include <set>
#include <utility>
#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"
#include "state_status.h"

class AbstractNode;
class ActionNode;
class ActionNodeHistory;
class PassThroughExperiment;
class ScopeHistory;
class Sequence;
class SequenceHistory;
class State;

const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 1;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW_PRE = 2;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 3;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 4;

const int BRANCH_EXPERIMENT_STATE_FAIL = 5;
const int BRANCH_EXPERIMENT_STATE_SUCCESS = 6;

class BranchExperimentOverallHistory;
class BranchExperiment : public AbstractExperiment {
public:
	PassThroughExperiment* parent_pass_through_experiment;

	double average_instances_per_run;
	/**
	 * - when triggering an experiment, it becomes live everywhere
	 *   - for one selected instance, always trigger branch and experiment
	 *     - for everywhere else, trigger accordingly
	 * 
	 * - set probabilities after average_instances_per_run to 50%
	 * 
	 * - average_instances_per_run changes between existing and new, as well as with new states
	 *   - so constantly best effort update
	 */

	int state;
	int state_iter;

	double existing_average_score;
	double existing_score_variance;

	std::set<std::pair<int, AbstractNode*>> s_possible_exits;
	std::vector<std::pair<int, AbstractNode*>> possible_exits;

	std::vector<std::map<int, double>> existing_input_state_weights;
	std::vector<std::map<int, double>> existing_local_state_weights;
	std::vector<std::map<State*, double>> existing_temp_state_weights;

	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<Sequence*> curr_sequences;
	int curr_exit_depth;
	AbstractNode* curr_exit_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<Sequence*> best_sequences;
	int best_exit_depth;
	AbstractNode* best_exit_node;

	double new_average_score;

	std::vector<std::map<int, double>> new_input_state_weights;
	std::vector<std::map<int, double>> new_local_state_weights;
	std::vector<std::map<State*, double>> new_temp_state_weights;

	std::vector<State*> new_states;
	std::vector<std::vector<AbstractNode*>> new_state_nodes;
	std::vector<std::vector<std::vector<int>>> new_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> new_state_node_contexts;
	std::vector<std::vector<int>> new_state_obs_indexes;

	double combined_score;
	int branch_count;
	int branch_possible;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_input_state_vals_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_local_state_vals_histories;
	std::vector<std::vector<std::map<State*, StateStatus>>> i_temp_state_vals_histories;
	std::vector<double> i_target_val_histories;

	BranchExperiment(std::vector<int> scope_context,
					 std::vector<int> node_context);
	~BranchExperiment();

	void activate(AbstractNode*& curr_node,
				  Problem& problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  AbstractExperimentHistory*& history);
	void hook(std::vector<ContextLayer>& context);
	void hook_helper(std::vector<int>& scope_context,
					 std::vector<int>& node_context,
					 std::map<State*, StateStatus>& temp_state_vals,
					 ScopeHistory* scope_history);
	void hook_experiment_helper(std::vector<int>& scope_context,
								std::vector<int>& node_context,
								std::map<State*, StateStatus>& temp_state_vals,
								AbstractExperimentHistory* experiment_history);
	void unhook();
	void parent_scope_end_activate(std::vector<ContextLayer>& context,
								   RunHelper& run_helper,
								   ScopeHistory* parent_scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper,
				  BranchExperimentOverallHistory* history);

	void train_existing_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void possible_exits_helper(int curr_exit_depth,
							   ScopeHistory* scope_history);
	void train_existing_parent_scope_end_activate(ScopeHistory* parent_scope_history);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper,
								 BranchExperimentOverallHistory* history);

	void explore_activate(AbstractNode*& curr_node,
						  Problem& problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper);
	void explore_target_activate(AbstractNode*& curr_node,
								 Problem& problem,
								 std::vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper);
	void explore_backprop(double target_val,
						  BranchExperimentOverallHistory* history);

	void train_new_activate(AbstractNode*& curr_node,
							Problem& problem,
							std::vector<ContextLayer>& context,
							int& exit_depth,
							AbstractNode*& exit_node,
							RunHelper& run_helper,
							AbstractExperimentHistory*& history);
	void train_new_target_activate(AbstractNode*& curr_node,
								   Problem& problem,
								   std::vector<ContextLayer>& context,
								   int& exit_depth,
								   AbstractNode*& exit_node,
								   RunHelper& run_helper);
	void train_new_non_target_activate(AbstractNode*& curr_node,
									   Problem& problem,
									   std::vector<ContextLayer>& context,
									   int& exit_depth,
									   AbstractNode*& exit_node,
									   RunHelper& run_helper,
									   AbstractExperimentHistory*& history);
	void train_new_backprop(double target_val,
							BranchExperimentOverallHistory* history);

	void measure_activate(AbstractNode*& curr_node,
						  Problem& problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper);
	void measure_backprop(double target_val);

	void finalize(std::map<std::pair<int, std::pair<bool,int>>, int>& input_scope_depths_mappings,
				  std::map<std::pair<int, std::pair<bool,int>>, int>& output_scope_depths_mappings);
	void new_branch(std::map<std::pair<int, std::pair<bool,int>>, int>& input_scope_depths_mappings,
					std::map<std::pair<int, std::pair<bool,int>>, int>& output_scope_depths_mappings);
	void new_pass_through(std::map<std::pair<int, std::pair<bool,int>>, int>& input_scope_depths_mappings,
						  std::map<std::pair<int, std::pair<bool,int>>, int>& output_scope_depths_mappings);
};

class BranchExperimentInstanceHistory : public AbstractExperimentHistory {
public:
	std::vector<void*> step_histories;

	BranchExperimentInstanceHistory(BranchExperiment* experiment);
	BranchExperimentInstanceHistory(BranchExperimentInstanceHistory* original);
	~BranchExperimentInstanceHistory();
};

class BranchExperimentOverallHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;
	double existing_predicted_score;

	BranchExperimentOverallHistory(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */