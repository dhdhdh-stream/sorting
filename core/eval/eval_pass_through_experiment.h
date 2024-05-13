#ifndef EVAL_PASS_THROUGH_EXPERIMENT_H
#define EVAL_PASS_THROUGH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class InfoScope;
class InfoScopeNode;
class Network;
class Problem;
class ScopeHistory;
class Solution;

const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE = 0;
const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING = 1;
const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 2;
const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW = 3;
const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE = 4;
const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 5;
const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST = 6;
const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 7;
const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND = 8;

class EvalPassThroughExperimentHistory;
class EvalPassThroughExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_standard_deviation;
	double existing_average_misguess;

	double new_score;
	InfoScope* info_scope;
	bool is_negate;
	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<InfoScopeNode*> scopes;
	AbstractNode* exit_next_node;

	ActionNode* ending_node;

	double new_average_score;

	std::vector<AbstractNode*> input_node_contexts;
	std::vector<int> input_obs_indexes;

	std::vector<double> linear_weights;
	std::vector<std::vector<int>> network_input_indexes;
	Network* network;
	double network_average_misguess;
	double network_misguess_standard_deviation;

	std::vector<double> misguess_histories;

	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;

	EvalPassThroughExperiment(AbstractNode* node_context,
							  bool is_branch);
	~EvalPassThroughExperiment();
	void decrement(AbstractNode* experiment_node);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void back_activate(Problem* problem,
					   ScopeHistory*& subscope_history,
					   RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void measure_existing_score_backprop(double target_val,
										 RunHelper& run_helper);

	void measure_existing_back_activate(ScopeHistory*& subscope_history,
										RunHelper& run_helper);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper);
	void explore_backprop(double target_val,
						  RunHelper& run_helper);

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							RunHelper& run_helper);
	void train_new_back_activate(ScopeHistory*& subscope_history,
								 RunHelper& run_helper);
	void train_new_backprop(double target_val,
							RunHelper& run_helper);

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper);
	void measure_back_activate(ScopeHistory*& subscope_history,
							   RunHelper& run_helper);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	void verify_existing_back_activate(ScopeHistory*& subscope_history,
									   RunHelper& run_helper);
	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper);
	void verify_back_activate(ScopeHistory*& subscope_history,
							  RunHelper& run_helper);
	void verify_backprop(double target_val,
						 RunHelper& run_helper);

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class EvalPassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	std::vector<double> predicted_scores;

	EvalPassThroughExperimentHistory(EvalPassThroughExperiment* experiment);
};

#endif /* EVAL_PASS_THROUGH_EXPERIMENT_H */