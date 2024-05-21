#ifndef EVAL_PASS_THROUGH_EXPERIMENT_H
#define EVAL_PASS_THROUGH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class Eval;
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

class EvalPassThroughExperimentHistory;
class EvalPassThroughExperiment : public AbstractExperiment {
public:
	Eval* eval_context;

	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

	double existing_score_average_misguess;
	double existing_score_misguess_standard_deviation;
	double existing_vs_average_misguess;
	double existing_vs_misguess_standard_deviation;

	double new_score;
	InfoScope* info_scope;
	bool is_negate;
	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<InfoScopeNode*> scopes;
	AbstractNode* exit_next_node;

	ActionNode* ending_node;

	double score_average_score;

	std::vector<AbstractNode*> score_input_node_contexts;
	std::vector<int> score_input_obs_indexes;

	std::vector<double> score_linear_weights;
	std::vector<std::vector<int>> score_network_input_indexes;
	Network* score_network;
	double score_network_average_misguess;
	double score_network_misguess_standard_deviation;

	double vs_average_score;

	std::vector<bool> vs_input_is_start;
	std::vector<AbstractNode*> vs_input_node_contexts;
	std::vector<int> vs_input_obs_indexes;

	std::vector<double> vs_linear_weights;
	std::vector<std::vector<int>> vs_network_input_indexes;
	Network* vs_network;
	double vs_network_average_misguess;
	double vs_network_misguess_standard_deviation;

	std::vector<double> score_misguess_histories;
	std::vector<double> vs_misguess_histories;

	std::vector<ScopeHistory*> start_scope_histories;
	std::vector<ScopeHistory*> end_scope_histories;
	std::vector<double> end_target_val_histories;

	EvalPassThroughExperiment(Eval* eval_context);
	~EvalPassThroughExperiment();
	void decrement(AbstractNode* experiment_node);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void backprop(EvalHistory* eval_history,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void measure_existing_score_backprop(Problem* problem,
										 std::vector<ContextLayer>& context,
										 RunHelper& run_helper);

	void measure_existing_backprop(EvalHistory* eval_history,
								   Problem* problem,
								   std::vector<ContextLayer>& context,
								   RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper);
	void explore_backprop(Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							RunHelper& run_helper);
	void train_new_backprop(EvalHistory* eval_history,
							Problem* problem,
							std::vector<ContextLayer>& context,
							RunHelper& run_helper);
	void train_score();
	void train_vs();

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper);
	void measure_backprop(EvalHistory* eval_history,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class EvalPassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	EvalHistory* outer_eval_history;

	EvalPassThroughExperimentHistory(EvalPassThroughExperiment* experiment);
	~EvalPassThroughExperimentHistory();
};

#endif /* EVAL_PASS_THROUGH_EXPERIMENT_H */