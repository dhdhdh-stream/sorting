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

const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 0;
const int EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE = 1;

class EvalPassThroughExperimentHistory;
class EvalPassThroughExperiment : public AbstractExperiment {
public:
	Eval* eval_context;

	int state;
	int state_iter;

	double new_score;
	InfoScope* info_scope;
	bool is_negate;
	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<InfoScopeNode*> scopes;
	AbstractNode* exit_next_node;

	ActionNode* ending_node;

	double average_score;

	std::vector<AbstractNode*> input_node_contexts;
	std::vector<int> input_obs_indexes;

	std::vector<double> linear_weights;
	std::vector<std::vector<int>> network_input_indexes;
	Network* network;
	double network_average_misguess;
	double network_misguess_standard_deviation;

	std::vector<EvalHistory*> eval_histories;

	std::vector<double> misguess_histories;

	EvalPassThroughExperiment(Eval* eval_context,
							  AbstractNode* node_context,
							  bool is_branch);
	~EvalPassThroughExperiment();
	void decrement(AbstractNode* experiment_node);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void backprop(EvalHistory* outer_eval_history,
				  EvalHistory* eval_history,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper);
	void explore_backprop(EvalHistory* outer_eval_history,
						  EvalHistory* eval_history,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);

	void train_new();

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper);
	void measure_backprop(EvalHistory* outer_eval_history,
						  EvalHistory* eval_history,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class EvalPassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	EvalPassThroughExperimentHistory(EvalPassThroughExperiment* experiment);
};

#endif /* EVAL_PASS_THROUGH_EXPERIMENT_H */