#ifndef RETRAIN_LOOP_EXPERIMENT_H
#define RETRAIN_LOOP_EXPERIMENT_H

#include <map>
#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class Problem;
class ScopeHistory;
class ScopeNode;
class ScopeNodeHistory;
class State;

const int RETRAIN_LOOP_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
/**
 * - learn halt with furthest existing
 */
const int RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_HALT = 1;
/**
 * - learn continue with plus one
 */
const int RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE_PRE = 2;
const int RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE = 3;
const int RETRAIN_LOOP_EXPERIMENT_STATE_MEASURE = 4;
const int RETRAIN_LOOP_EXPERIMENT_STATE_VERIFY_EXISTING = 5;
const int RETRAIN_LOOP_EXPERIMENT_STATE_VERIFY = 6;
#if defined(MDEBUG) && MDEBUG
const int RETRAIN_LOOP_EXPERIMENT_STATE_CAPTURE_VERIFY = 7;
#endif /* MDEBUG */

const int RETRAIN_LOOP_EXPERIMENT_STATE_FAIL = 8;
const int RETRAIN_LOOP_EXPERIMENT_STATE_SUCCESS = 9;

class RetrainLoopExperimentOverallHistory;
class RetrainLoopExperiment : public AbstractExperiment {
public:
	ScopeNode* scope_node;

	double average_instances_per_run;

	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_variance;
	double existing_standard_deviation;

	std::vector<std::map<int, double>> halt_input_state_weights;
	std::vector<std::map<int, double>> halt_local_state_weights;
	std::vector<std::map<State*, double>> halt_temp_state_weights;

	std::vector<std::map<int, double>> continue_input_state_weights;
	std::vector<std::map<int, double>> continue_local_state_weights;
	std::vector<std::map<State*, double>> continue_temp_state_weights;

	std::vector<State*> new_states;
	std::vector<std::vector<ActionNode*>> new_state_nodes;
	std::vector<std::vector<std::vector<int>>> new_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> new_state_node_contexts;
	std::vector<std::vector<int>> new_state_obs_indexes;

	double combined_score;
	int plus_one_count;

	double verify_existing_average_score;
	double verify_existing_score_variance;

	std::vector<double> o_target_val_histories;

	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_input_state_vals_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_local_state_vals_histories;
	std::vector<std::vector<std::map<State*, StateStatus>>> i_temp_state_vals_histories;
	std::vector<double> i_target_val_histories;
	/**
	 * - for continue, capture then overwrite if better
	 */

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_continue_scores;
	std::vector<double> verify_halt_scores;
	std::vector<std::vector<double>> verify_factors;
	#endif /* MDEBUG */

	RetrainLoopExperiment(ScopeNode* scope_node);
	~RetrainLoopExperiment();

	bool activate(Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& inner_exit_depth,
				  AbstractNode*& inner_exit_node,
				  RunHelper& run_helper,
				  ScopeNodeHistory* parent_scope_node_history);
	void hook();
	void back_activate_helper(std::vector<int>& scope_context,
							  std::vector<int>& node_context,
							  std::map<State*, StateStatus>& temp_state_vals,
							  ScopeHistory* scope_history);
	void back_activate(std::vector<ContextLayer>& context);
	void unhook();
	void backprop(double target_val,
				  RunHelper& run_helper,
				  RetrainLoopExperimentOverallHistory* history);

	void measure_existing_activate(Problem* problem,
								   std::vector<ContextLayer>& context,
								   int& inner_exit_depth,
								   AbstractNode*& inner_exit_node,
								   RunHelper& run_helper,
								   ScopeNodeHistory* parent_scope_node_history);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void train_halt_activate(Problem* problem,
							 std::vector<ContextLayer>& context,
							 int& inner_exit_depth,
							 AbstractNode*& inner_exit_node,
							 RunHelper& run_helper,
							 ScopeNodeHistory* parent_scope_node_history);
	void train_halt_backprop(double target_val,
							 RetrainLoopExperimentOverallHistory* history);

	void train_continue_activate(Problem* problem,
								 std::vector<ContextLayer>& context,
								 int& inner_exit_depth,
								 AbstractNode*& inner_exit_node,
								 RunHelper& run_helper,
								 ScopeNodeHistory* parent_scope_node_history);
	void train_continue_target_activate(Problem* problem,
										std::vector<ContextLayer>& context,
										int& inner_exit_depth,
										AbstractNode*& inner_exit_node,
										RunHelper& run_helper,
										ScopeNodeHistory* parent_scope_node_history);
	void train_continue_non_target_activate(Problem* problem,
											std::vector<ContextLayer>& context,
											int& inner_exit_depth,
											AbstractNode*& inner_exit_node,
											RunHelper& run_helper,
											ScopeNodeHistory* parent_scope_node_history);
	void train_continue_backprop(double target_val,
								 RetrainLoopExperimentOverallHistory* history);

	void measure_activate(Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& inner_exit_depth,
						  AbstractNode*& inner_exit_node,
						  RunHelper& run_helper,
						  ScopeNodeHistory* parent_scope_node_history);
	void measure_backprop(double target_val);

	void verify_existing_activate(Problem* problem,
								  std::vector<ContextLayer>& context,
								  int& inner_exit_depth,
								  AbstractNode*& inner_exit_node,
								  RunHelper& run_helper,
								  ScopeNodeHistory* parent_scope_node_history);
	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_activate(Problem* problem,
						 std::vector<ContextLayer>& context,
						 int& inner_exit_depth,
						 AbstractNode*& inner_exit_node,
						 RunHelper& run_helper,
						 ScopeNodeHistory* parent_scope_node_history);
	void verify_backprop(double target_val);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(Problem* problem,
								 std::vector<ContextLayer>& context,
								 int& inner_exit_depth,
								 AbstractNode*& inner_exit_node,
								 RunHelper& run_helper,
								 ScopeNodeHistory* parent_scope_node_history);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void finalize();
};

class RetrainLoopExperimentOverallHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;
	int num_iters;

	RetrainLoopExperimentOverallHistory(RetrainLoopExperiment* experiment);
};

#endif /* RETRAIN_LOOP_EXPERIMENT_H */