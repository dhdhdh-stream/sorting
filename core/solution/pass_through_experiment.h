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
class InfoBranchNode;
class InfoScope;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 1;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_EXISTING = 2;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_NEW = 3;
const int PASS_THROUGH_EXPERIMENT_STATE_ROOT_VERIFY = 4;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT = 5;

class PassThroughExperimentHistory;
class PassThroughExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;
	int explore_iter;

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
	InfoBranchNode* info_branch_node;

	PassThroughExperiment(AbstractScope* scope_context,
						  AbstractNode* node_context,
						  bool is_branch,
						  int score_type,
						  AbstractExperiment* parent_experiment);
	~PassThroughExperiment();
	void decrement(AbstractNode* experiment_node);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void back_activate(std::vector<ContextLayer>& context,
					   RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void measure_existing_activate(std::vector<ContextLayer>& context,
								   PassThroughExperimentHistory* history);
	void measure_existing_back_activate(std::vector<ContextLayer>& context,
										RunHelper& run_helper);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  PassThroughExperimentHistory* history);
	void explore_back_activate(std::vector<ContextLayer>& context,
							   RunHelper& run_helper);
	void explore_backprop(double target_val,
						  RunHelper& run_helper);

	void verify_existing_activate(std::vector<ContextLayer>& context,
								  PassThroughExperimentHistory* history);
	void verify_existing_back_activate(std::vector<ContextLayer>& context,
									   RunHelper& run_helper);
	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_new_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 PassThroughExperimentHistory* history);
	void verify_new_back_activate(std::vector<ContextLayer>& context,
								  RunHelper& run_helper);
	void verify_new_backprop(double target_val,
							 RunHelper& run_helper);

	void root_verify_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  std::vector<ContextLayer>& context,
							  RunHelper& run_helper);

	void experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 PassThroughExperimentHistory* history);
	void experiment_back_activate(std::vector<ContextLayer>& context,
								  RunHelper& run_helper);
	void experiment_backprop(RunHelper& run_helper);

	void experiment_verify_existing_activate(std::vector<ContextLayer>& context,
											 PassThroughExperimentHistory* history);
	void experiment_verify_existing_back_activate(std::vector<ContextLayer>& context,
												  RunHelper& run_helper);
	void experiment_verify_existing_backprop(double target_val,
											 RunHelper& run_helper);

	void experiment_verify_new_activate(AbstractNode*& curr_node,
										Problem* problem,
										std::vector<ContextLayer>& context,
										RunHelper& run_helper,
										PassThroughExperimentHistory* history);
	void experiment_verify_new_back_activate(std::vector<ContextLayer>& context,
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
};

#endif /* PASS_THROUGH_EXPERIMENT_H */