/**
 * - branch experiment
 *   - no need to worry about passthroughs as would be more relevantly handled by eval sequence
 */

#ifndef ORIENTATION_EXPERIMENT_H
#define ORIENTATION_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class Eval;
class EvalHistory;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;

const int ORIENTATION_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int ORIENTATION_EXPERIMENT_STATE_EXPLORE_MISGUESS = 1;
/**
 * - also gathers samples to train new
 */
const int ORIENTATION_EXPERIMENT_STATE_EXPLORE_IMPACT = 2;
const int ORIENTATION_EXPERIMENT_STATE_MEASURE = 3;
const int ORIENTATION_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 4;
const int ORIENTATION_EXPERIMENT_STATE_VERIFY_1ST = 5;
const int ORIENTATION_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 6;
const int ORIENTATION_EXPERIMENT_STATE_VERIFY_2ND = 7;
#if defined(MDEBUG) && MDEBUG
const int ORIENTATION_EXPERIMENT_STATE_CAPTURE_VERIFY = 8;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int ORIENTATION_EXPERIMENT_EXPLORE_ITERS = 2;
#else
const int ORIENTATION_EXPERIMENT_EXPLORE_ITERS = 100;
#endif /* MDEBUG */

class OrientationExperimentHistory;
class OrientationExperiment : public AbstractExperiment {
public:
	Eval* eval_context;

	int state;
	int state_iter;
	int explore_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;
	std::vector<int> input_obs_indexes;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_average_misguess;
	double existing_misguess_standard_deviation;

	double new_score;
	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<ScopeNode*> scopes;
	AbstractNode* exit_next_node;

	ActionNode* ending_node;

	double new_average_score;

	std::vector<double> new_linear_weights;
	std::vector<std::vector<int>> new_network_input_indexes;
	Network* new_network;
	double new_average_misguess;
	double new_misguess_standard_deviation;

	double combined_score;
	int original_count;
	int branch_count;
	double branch_weight;

	double verify_existing_average_score;

	bool is_pass_through;

	std::vector<ScopeHistory*> scope_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	#endif /* MDEBUG */

	OrientationExperiment(Eval* eval_context,
						  AbstractNode* node_context,
						  bool is_branch);
	~OrientationExperiment();
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

	void train_existing_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void train_existing_backprop(EvalHistory* outer_eval_history,
								 EvalHistory* eval_history,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);

	void explore_misguess_activate(AbstractNode*& curr_node,
								   Problem* problem,
								   std::vector<ContextLayer>& context,
								   RunHelper& run_helper,
								   OrientationExperimentHistory* history);
	void explore_misguess_backprop(EvalHistory* outer_eval_history,
								   EvalHistory* eval_history,
								   Problem* problem,
								   std::vector<ContextLayer>& context,
								   RunHelper& run_helper);

	void explore_impact_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void explore_impact_backprop(EvalHistory* outer_eval_history,
								 EvalHistory* eval_history,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);

	void train_new();

	bool measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);
	void measure_backprop(EvalHistory* outer_eval_history,
						  EvalHistory* eval_history,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);

	void verify_existing_backprop(EvalHistory* outer_eval_history,
								  EvalHistory* eval_history,
								  Problem* problem,
								  std::vector<ContextLayer>& context,
								  RunHelper& run_helper);

	bool verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	void verify_backprop(EvalHistory* outer_eval_history,
						 EvalHistory* eval_history,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	bool capture_verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class OrientationExperimentHistory : public AbstractExperimentHistory {
public:
	double existing_predicted_score;

	OrientationExperimentHistory(OrientationExperiment* experiment);
};

#endif /* ORIENTATION_EXPERIMENT_H */