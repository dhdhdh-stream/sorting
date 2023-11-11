#ifndef PASS_THROUGH_EXPERIMENT_H
#define PASS_THROUGH_EXPERIMENT_H

const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE = 0;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 1;
/**
 * - simply choose highest score
 */
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_SCORE = 2;
/**
 * - if positive, can simply add and skip rest
 */
/**
 * - don't train existing new state beyond MEASURE_EXISTING_SCORE
 */
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS = 3;
const int PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW_MISGUESS = 4;
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_MISGUESS = 5;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT = 6;

const int PASS_THROUGH_EXPERIMENT_STATE_FAIL = 7;
const int PASS_THROUGH_EXPERIMENT_STATE_SUCCESS = 8;

class PassThroughExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_variance;

	std::vector<std::pair<int, AbstractNode*>> possible_exits;

	double curr_score;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<Sequence*> curr_sequences;
	int curr_exit_depth;
	int curr_exit_node_id;

	double best_score;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<Sequence*> best_sequences;
	int best_exit_depth;
	int best_exit_node_id;

	double new_average_score;

	std::map<int, double> existing_input_state_weights;
	std::map<int, double> existing_local_state_weights;
	std::map<State*, double> existing_temp_state_weights;

	/**
	 * - measure using sqr over abs
	 *   - even though sqr may not measure true score improvement, it measures information improvement
	 *     - which ultimately leads to better branching
	 */
	double existing_average_misguess;
	double existing_misguess_variance;

	std::vector<std::map<int, double>> new_input_state_weights;
	std::vector<std::map<int, double>> new_local_state_weights;
	std::vector<std::map<State*, double>> new_temp_state_weights;

	double new_average_misguess;
	/**
	 * - won't be exact comparison, but only meant to support followup BranchExperiments
	 *   - e.g., instances may be different due to path change, recursion
	 */

	std::map<AbstractNode*, int> node_to_step_index;

	std::vector<State*> new_states;
	std::vector<std::vector<AbstractNode*>> new_state_nodes;
	std::vector<std::vector<std::vector<int>>> new_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> new_state_node_contexts;
	std::vector<std::vector<int>> new_state_obs_indexes;
	/**
	 * - upon success, add all to parent scope and handle from there
	 */

	std::vector<double> o_target_val_histories;

	std::vector<ScopeHistory*> i_scope_histories;
	/**
	 * - simply save ScopeHistorys, instead of preprocessing them for ObsExperiment
	 *   - not as efficient, but shouldn't be a big deal
	 *     - would have to backtrack through anyways on experiment select
	 */
	std::vector<std::vector<std::map<int, StateStatus>>> i_input_state_vals_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_local_state_vals_histories;
	std::vector<std::vector<std::map<State*, StateStatus>>> i_temp_state_vals_histories;
	std::vector<double> i_target_val_histories;

	std::vector<double> i_misguess_histories;

	int branch_experiment_step_index;
	BranchExperiment* branch_experiment;

	PassThroughExperiment(std::vector<int> scope_context,
						  std::vector<int> node_context);
	~PassThroughExperiment();

	void activate(AbstractNode*& curr_node,
				  Problem& problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  AbstractExperimentHistory*& history);
	void hook(std::vector<ContextLayer>& context);
	void unhook();
	void parent_scope_end_activate(std::vector<ContextLayer>& context,
								   RunHelper& run_helper,
								   ScopeHistory* parent_scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper,
				  PassThroughExperimentOverallHistory* history);

	void measure_existing_score_activate(std::vector<ContextLayer>& context);
	void measure_existing_score_parent_scope_end_activate(
		std::vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* parent_scope_history);
	void measure_existing_score_backprop(double target_val,
										 RunHelper& run_helper,
										 PassThroughExperimentOverallHistory* history);

	void explore_initial_activate(AbstractNode*& curr_node,
								  Problem& problem,
								  std::vector<ContextLayer>& context,
								  int& exit_depth,
								  AbstractNode*& exit_node,
								  RunHelper& run_helper);
	void explore_activate(AbstractNode*& curr_node,
						  Problem& problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper);
	void explore_backprop(double target_val);

	void measure_new_score_activate(AbstractNode*& curr_node,
									Problem& problem,
									std::vector<ContextLayer>& context,
									int& exit_depth,
									AbstractNode*& exit_node,
									RunHelper& run_helper,
									AbstractExperimentHistory*& history);
	void measure_new_score_backprop(double target_val,
									PassThroughExperimentOverallHistory* history);

	void measure_existing_misguess_activate(std::vector<ContextLayer>& context);
	void measure_existing_misguess_parent_scope_end_activate(
		std::vector<ContextLayer>& context,
		RunHelper& run_helper);
	void measure_existing_misguess_backprop(double target_val,
											RunHelper& run_helper,
											PassThroughExperimentOverallHistory* history);

	void train_new_misguess_activate(AbstractNode*& curr_node,
									 Problem& problem,
									 std::vector<ContextLayer>& context,
									 int& exit_depth,
									 AbstractNode*& exit_node,
									 RunHelper& run_helper,
									 AbstractExperimentHistory*& history);
	void train_new_misguess_backprop(double target_val,
									 PassThroughExperimentOverallHistory* history);

	void measure_new_misguess_activate(AbstractNode*& curr_node,
									   Problem& problem,
									   std::vector<ContextLayer>& context,
									   int& exit_depth,
									   AbstractNode*& exit_node,
									   RunHelper& run_helper,
									   AbstractExperimentHistory*& history);
	void measure_new_misguess_backprop(double target_val,
									   PassThroughExperimentOverallHistory* history);

	void experiment_activate(AbstractNode*& curr_node,
							 Problem& problem,
							 std::vector<ContextLayer>& context,
							 int& exit_depth,
							 AbstractNode*& exit_node,
							 RunHelper& run_helper,
							 AbstractExperimentHistory*& history);
	void experiment_backprop(double target_val,
							 PassThroughExperimentOverallHistory* history);
};

class PassThroughExperimentInstanceHistory : public AbstractExperimentHistory {
public:
	PassThroughExperiment* experiment;

	std::vector<void*> pre_step_histories;

	BranchExperimentInstanceHistory* branch_experiment_history;

	std::vector<void*> post_step_histories;

	PassThroughExperimentInstanceHistory(PassThroughExperiment* experiment);
	PassThroughExperimentInstanceHistory(PassThroughExperimentInstanceHistory* original);
	~PassThroughExperimentInstanceHistory();
};

class PassThroughExperimentOverallHistory : public AbstractExperimentHistory {
public:
	PassThroughExperiment* experiment;

	int instance_count;
	std::vector<double> predicted_scores;

	BranchExperimentOverallHistory* branch_experiment_history;

	PassThroughExperimentOverallHistory(PassThroughExperiment* experiment);
	~PassThroughExperimentOverallHistory();
};

#endif /* PASS_THROUGH_EXPERIMENT_H */