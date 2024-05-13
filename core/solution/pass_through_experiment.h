#ifndef PASS_THROUGH_EXPERIMENT_H
#define PASS_THROUGH_EXPERIMENT_H

#include <map>
#include <set>
#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class InfoScope;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 1;
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW = 2;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 3;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW = 4;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 5;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_NEW = 6;
const int PASS_THROUGH_EXPERIMENT_STATE_ROOT_VERIFY = 7;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT = 8;

#if defined(MDEBUG) && MDEBUG
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 2;
#else
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 100;
#endif /* MDEBUG */

class PassThroughExperimentHistory;
class PassThroughExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;

	double curr_score;
	InfoScope* curr_info_scope;
	bool curr_is_negate;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_score;
	InfoScope* best_info_scope;
	bool best_is_negate;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_scopes;
	AbstractNode* best_exit_next_node;

	ActionNode* ending_node;

	PassThroughExperiment(Scope* scope_context,
						  AbstractNode* node_context,
						  bool is_branch,
						  AbstractExperiment* parent_experiment);
	~PassThroughExperiment();
	void decrement(AbstractNode* experiment_node);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void measure_existing_activate(PassThroughExperimentHistory* history);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);
	void explore_backprop(double target_val,
						  RunHelper& run_helper);

	void measure_new_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  RunHelper& run_helper);
	void measure_new_backprop(double target_val,
							  RunHelper& run_helper);

	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_new_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 RunHelper& run_helper);
	void verify_new_backprop(double target_val,
							 RunHelper& run_helper);

	void root_verify_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  RunHelper& run_helper);

	void experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 PassThroughExperimentHistory* history);
	void experiment_backprop(double target_val,
							 RunHelper& run_helper);

	void experiment_verify_existing_backprop(double target_val,
											 RunHelper& run_helper);

	void experiment_verify_new_activate(AbstractNode*& curr_node,
										Problem* problem,
										RunHelper& run_helper);
	void experiment_verify_new_backprop(double target_val,
										RunHelper& run_helper);

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class PassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;

	PassThroughExperimentHistory(PassThroughExperiment* experiment);
	~PassThroughExperimentHistory();
};

#endif /* PASS_THROUGH_EXPERIMENT_H */