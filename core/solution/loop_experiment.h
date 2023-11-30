#ifndef LOOP_EXPERIMENT_H
#define LOOP_EXPERIMENT_H

#include <map>
#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"
#include "state_status.h"

class AbstractNode;
class ActionNode;
class PotentialScopeNode;
class PotentialScopeNodeHistory;
class ScopeHistory;
class State;

const int LOOP_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int LOOP_EXPERIMENT_STATE_EXPLORE = 1;

const int LOOP_EXPERIMENT_STATE_TRAIN_PRE = 2;
const int LOOP_EXPERIMENT_STATE_TRAIN = 3;
const int LOOP_EXPERIMENT_SUB_STATE_TRAIN_HALT = 0;
const int LOOP_EXPERIMENT_SUB_STATE_TRAIN_CONTINUE = 1;

const int LOOP_EXPERIMENT_STATE_MEASURE = 4;
const int LOOP_EXPERIMENT_STATE_VERIFY_EXISTING = 5;
const int LOOP_EXPERIMENT_STATE_VERIFY = 6;
const int LOOP_EXPERIMENT_STATE_CAPTURE_VERIFY = 7;

const int LOOP_EXPERIMENT_STATE_FAIL = 8;
const int LOOP_EXPERIMENT_STATE_SUCCESS = 9;

/**
 * - don't train/evaluate past limit to see if has better score
 *   - so won't train to go past limit (to receive -1.0 score)
 *     - but hopefully, after finalized, can generalize to go past limit when correct
 */
const int TRAIN_ITER_LIMIT = 8;
const int NUM_SAMPLES_MULTIPLIER = 2;

class LoopExperimentOverallHistory;
class LoopExperiment : public AbstractExperiment {
public:
	double average_instances_per_run;

	int state;
	int sub_state;
	int state_iter;
	int sub_state_iter;

	PotentialScopeNode* potential_loop;

	double existing_average_score;
	double existing_score_variance;

	std::vector<std::map<int, double>> start_input_state_weights;
	std::vector<std::map<int, double>> start_local_state_weights;
	std::vector<std::map<State*, double>> start_temp_state_weights;

	std::vector<std::map<int, double>> halt_input_state_weights;
	std::vector<std::map<int, double>> halt_local_state_weights;
	std::vector<std::map<State*, double>> halt_temp_state_weights;
	int halt_constant;

	std::vector<std::map<int, double>> continue_input_state_weights;
	std::vector<std::map<int, double>> continue_local_state_weights;
	std::vector<std::map<State*, double>> continue_temp_state_weights;
	int continue_constant;

	std::vector<State*> new_states;
	std::vector<std::vector<ActionNode*>> new_state_nodes;
	std::vector<std::vector<std::vector<int>>> new_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> new_state_node_contexts;
	std::vector<std::vector<int>> new_state_obs_indexes;

	double measure_score;
	int measure_num_instances;
	int measure_sum_iters;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_input_state_vals_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_local_state_vals_histories;
	std::vector<std::vector<std::map<State*, StateStatus>>> i_temp_state_vals_histories;
	std::vector<double> i_target_val_histories;
	std::vector<double> i_start_predicted_score_histories;

	std::vector<Problem> verify_problems;
	std::vector<double> verify_continue_scores;
	std::vector<double> verify_halt_scores;
	std::vector<std::vector<double>> verify_factors;
	std::vector<bool> verify_decision_is_halt;

	LoopExperiment(std::vector<int> scope_context,
				   std::vector<int> node_context,
				   PotentialScopeNode* potential_loop);
	~LoopExperiment();

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
	void unhook();
	void backprop(double target_val,
				  RunHelper& run_helper,
				  LoopExperimentOverallHistory* history);

	void train_existing_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper,
								 LoopExperimentOverallHistory* history);

	void explore_activate(Problem& problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);
	void explore_target_activate(Problem& problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void explore_backprop(double target_val,
						  LoopExperimentOverallHistory* history);

	void train_halt_activate(Problem& problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 AbstractExperimentHistory*& history);
	void train_halt_target_activate(Problem& problem,
									std::vector<ContextLayer>& context,
									RunHelper& run_helper,
									AbstractExperimentHistory*& history);
	void train_halt_non_target_activate(Problem& problem,
										std::vector<ContextLayer>& context,
										RunHelper& run_helper,
										AbstractExperimentHistory*& history);
	void train_halt_backprop(double target_val,
							 LoopExperimentOverallHistory* history);

	void train_continue_activate(Problem& problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 AbstractExperimentHistory*& history);
	void train_continue_target_activate(Problem& problem,
										std::vector<ContextLayer>& context,
										RunHelper& run_helper,
										AbstractExperimentHistory*& history);
	void train_continue_non_target_activate(Problem& problem,
											std::vector<ContextLayer>& context,
											RunHelper& run_helper,
											AbstractExperimentHistory*& history);
	void train_continue_backprop(double target_val,
								 LoopExperimentOverallHistory* history);

	void measure_activate(Problem& problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);
	void measure_backprop(double target_val);

	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_activate(Problem& problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	void verify_backprop(double target_val);

	void capture_verify_activate(Problem& problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void capture_verify_backprop();
};

class LoopExperimentInstanceHistory : public AbstractExperimentHistory {
public:
	std::vector<PotentialScopeNodeHistory*> iter_histories;

	LoopExperimentInstanceHistory(LoopExperiment* experiment);
	LoopExperimentInstanceHistory(LoopExperimentInstanceHistory* original);
	~LoopExperimentInstanceHistory();
};

class LoopExperimentOverallHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;
	double start_predicted_score;
	std::vector<double> halt_predicted_scores;

	LoopExperimentOverallHistory(LoopExperiment* experiment);
};

#endif /* LOOP_EXPERIMENT_H */