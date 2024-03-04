#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class ExitNode;
class Network;
class PassThroughExperiment;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 1;
/**
 * - go to TRAIN_NEW from EXPLORE 
 *   - i.e., skip 1st RETRAIN_EXISTING
 * 
 * - share sub_state_iter
 */
const int BRANCH_EXPERIMENT_STATE_RETRAIN_EXISTING = 2;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 3;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 4;
/**
 * - if has parent_pass_through_experiment, skip and verify in parent
 */
const int BRANCH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 5;
const int BRANCH_EXPERIMENT_STATE_VERIFY_1ST = 6;
const int BRANCH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 7;
const int BRANCH_EXPERIMENT_STATE_VERIFY_2ND = 8;
#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 9;
#endif /* MDEBUG */
const int BRANCH_EXPERIMENT_STATE_ROOT_VERIFY = 10;

const int BRANCH_EXPERIMENT_TRAIN_ITERS = 4;

class BranchExperimentHistory;
class BranchExperiment : public AbstractExperiment {
public:
	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;
	bool is_branch;
	int throw_id;

	PassThroughExperiment* parent_experiment;
	PassThroughExperiment* root_experiment;

	double average_instances_per_run;

	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_average_misguess;
	double existing_misguess_standard_deviation;

	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_existing_scopes;
	std::vector<ScopeNode*> curr_potential_scopes;
	bool curr_exit_is_throw;
	int curr_exit_depth;
	AbstractNode* curr_exit_next_node;
	/**
	 * - can reuse existing
	 *   - search current context for throw_ids
	 */
	int curr_exit_throw_id;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_existing_scopes;
	std::vector<ScopeNode*> best_potential_scopes;
	int best_exit_depth;
	AbstractNode* best_exit_next_node;
	ExitNode* exit_node;

	double new_average_score;

	std::vector<double> new_linear_weights;
	std::vector<std::vector<int>> new_network_input_indexes;
	Network* new_network;
	double new_average_misguess;
	double new_misguess_standard_deviation;

	double combined_score;
	/**
	 * - if original_count is 0, then is_pass_through
	 *   - simply use latest original_count
	 */
	int original_count;
	int branch_count;

	/**
	 * - don't reuse previous to not affect decision making
	 */
	double verify_existing_average_score;
	double verify_existing_score_standard_deviation;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	#endif /* MDEBUG */

	BranchExperiment(std::vector<Scope*> scope_context,
					 std::vector<AbstractNode*> node_context,
					 bool is_branch,
					 int throw_id,
					 PassThroughExperiment* parent_experiment);
	~BranchExperiment();

	bool activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void train_existing_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 BranchExperimentHistory* history);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper,
								 BranchExperimentHistory* history);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  BranchExperimentHistory* history);
	void explore_target_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper,
								 BranchExperimentHistory* history);
	void explore_backprop(double target_val,
						  BranchExperimentHistory* history);

	void retrain_existing_activate(AbstractNode*& curr_node,
								   Problem* problem,
								   std::vector<ContextLayer>& context,
								   int& exit_depth,
								   AbstractNode*& exit_node,
								   RunHelper& run_helper,
								   BranchExperimentHistory* history);
	void retrain_existing_target_activate(AbstractNode*& curr_node,
										  Problem* problem,
										  std::vector<ContextLayer>& context,
										  int& exit_depth,
										  AbstractNode*& exit_node,
										  RunHelper& run_helper);
	void retrain_existing_non_target_activate(AbstractNode*& curr_node,
											  Problem* problem,
											  std::vector<ContextLayer>& context,
											  int& exit_depth,
											  AbstractNode*& exit_node,
											  RunHelper& run_helper);
	void retrain_existing_backprop(double target_val,
								   BranchExperimentHistory* history);

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							std::vector<ContextLayer>& context,
							int& exit_depth,
							AbstractNode*& exit_node,
							RunHelper& run_helper,
							BranchExperimentHistory* history);
	void train_new_target_activate(AbstractNode*& curr_node,
								   Problem* problem,
								   std::vector<ContextLayer>& context,
								   int& exit_depth,
								   AbstractNode*& exit_node,
								   RunHelper& run_helper);
	void train_new_non_target_activate(AbstractNode*& curr_node,
									   Problem* problem,
									   std::vector<ContextLayer>& context,
									   int& exit_depth,
									   AbstractNode*& exit_node,
									   RunHelper& run_helper);
	void train_new_backprop(double target_val,
							BranchExperimentHistory* history);

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper);
	void verify_backprop(double target_val,
						 RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void root_verify_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  std::vector<ContextLayer>& context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  RunHelper& run_helper);

	void finalize();
	void new_branch();
	void new_pass_through();
};

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;
	double existing_predicted_score;

	BranchExperimentHistory(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */