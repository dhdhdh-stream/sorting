#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <set>
#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class BranchNode;
class InfoBranchNode;
class InfoScope;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 1;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 2;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 3;
const int BRANCH_EXPERIMENT_STATE_VERIFY_EXISTING = 4;
const int BRANCH_EXPERIMENT_STATE_VERIFY = 5;
#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 6;
#endif /* MDEBUG */
const int BRANCH_EXPERIMENT_STATE_ROOT_VERIFY = 7;
const int BRANCH_EXPERIMENT_STATE_EXPERIMENT = 8;

class BranchExperimentHistory;
class BranchExperiment : public AbstractExperiment {
public:
	int num_instances_until_target;

	int state;
	int state_iter;
	int sub_state_iter;
	int explore_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

	std::vector<std::vector<AbstractScope*>> existing_input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> existing_input_node_contexts;
	std::vector<int> existing_input_obs_indexes;
	Network* existing_network;

	int explore_type;

	/**
	 * TODO: chain InfoScopes
	 */
	InfoScope* curr_info_scope;
	bool curr_is_negate;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	InfoScope* best_info_scope;
	bool best_is_negate;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_scopes;
	AbstractNode* best_exit_next_node;

	BranchNode* branch_node;
	InfoBranchNode* info_branch_node;
	ActionNode* ending_node;

	double new_average_score;

	std::vector<std::vector<AbstractScope*>> new_input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> new_input_node_contexts;
	std::vector<int> new_input_obs_indexes;
	Network* new_network;

	double combined_score;
	int original_count;
	int branch_count;
	double branch_weight;

	bool is_pass_through;

	std::vector<AbstractScopeHistory*> scope_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	BranchExperiment(AbstractScope* scope_context,
					 AbstractNode* node_context,
					 bool is_branch,
					 int score_type,
					 AbstractExperiment* parent_experiment);
	~BranchExperiment();
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

	void train_existing_activate(std::vector<ContextLayer>& context,
								 BranchExperimentHistory* history);
	void train_existing_back_activate(std::vector<ContextLayer>& context,
									  RunHelper& run_helper);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper);

	bool explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  BranchExperimentHistory* history);
	void explore_back_activate(std::vector<ContextLayer>& context,
							   RunHelper& run_helper);
	void explore_backprop(double target_val,
						  RunHelper& run_helper);

	bool train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							std::vector<ContextLayer>& context,
							RunHelper& run_helper,
							BranchExperimentHistory* history);
	void train_new_back_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void train_new_backprop(double target_val,
							RunHelper& run_helper);

	bool measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  BranchExperimentHistory* history);
	void measure_back_activate(std::vector<ContextLayer>& context,
							   RunHelper& run_helper);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	void verify_existing_activate(std::vector<ContextLayer>& context,
								  BranchExperimentHistory* history);
	void verify_existing_back_activate(std::vector<ContextLayer>& context,
									   RunHelper& run_helper);
	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	bool verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 BranchExperimentHistory* history);
	void verify_back_activate(std::vector<ContextLayer>& context,
							  RunHelper& run_helper);
	void verify_backprop(double target_val,
						 RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	bool capture_verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	bool root_verify_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  std::vector<ContextLayer>& context,
							  RunHelper& run_helper);

	bool experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 BranchExperimentHistory* history);
	void experiment_back_activate(std::vector<ContextLayer>& context,
								  RunHelper& run_helper);
	void experiment_backprop(RunHelper& run_helper);

	void experiment_verify_existing_activate(std::vector<ContextLayer>& context,
											 BranchExperimentHistory* history);
	void experiment_verify_existing_back_activate(std::vector<ContextLayer>& context,
												  RunHelper& run_helper);
	void experiment_verify_existing_backprop(double target_val,
											 RunHelper& run_helper);

	bool experiment_verify_activate(AbstractNode*& curr_node,
									Problem* problem,
									std::vector<ContextLayer>& context,
									RunHelper& run_helper,
									BranchExperimentHistory* history);
	void experiment_verify_back_activate(std::vector<ContextLayer>& context,
										 RunHelper& run_helper);
	void experiment_verify_backprop(double target_val,
									RunHelper& run_helper);

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
	void new_existing_info(Solution* duplicate);
};

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;

	std::vector<double> existing_predicted_scores;

	BranchExperimentHistory(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */