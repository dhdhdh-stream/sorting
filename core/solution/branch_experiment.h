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
class ExitNode;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 1;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 2;
/**
 * - don't worry about retraining with new decision making
 *   - more likely to cause thrasing than to actually be helpful
 *   - simply hope that things work out, and if not, will be caught by MEASURE
 */
const int BRANCH_EXPERIMENT_STATE_MEASURE = 3;
const int BRANCH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 4;
const int BRANCH_EXPERIMENT_STATE_VERIFY_1ST = 5;
const int BRANCH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 6;
const int BRANCH_EXPERIMENT_STATE_VERIFY_2ND = 7;
#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 8;
#endif /* MDEBUG */
const int BRANCH_EXPERIMENT_STATE_ROOT_VERIFY = 9;
const int BRANCH_EXPERIMENT_STATE_EXPERIMENT = 10;

/**
 * - select first that is significant improvement
 *   - don't select "best" as might not have been learned for actual best
 *     - so may select lottery instead of actual best
 */
const int EXPLORE_TYPE_GOOD = 0;
const int EXPLORE_TYPE_NEUTRAL = 1;
const int EXPLORE_TYPE_BEST = 2;

const int MAX_EXPLORE_TRIES = 4;

const double EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT = 0.05;
const double PASS_THROUGH_BRANCH_WEIGHT = 0.9;

class BranchExperimentHistory;
class BranchExperiment : public AbstractExperiment {
public:
	bool skip_explore;

	int num_instances_until_target;

	int state;
	int state_iter;
	int explore_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;
	std::vector<int> input_obs_indexes;
	int input_max_depth;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_average_misguess;
	double existing_misguess_standard_deviation;

	int explore_type;

	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_scopes;
	int curr_exit_depth;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_scopes;
	int best_exit_depth;
	AbstractNode* best_exit_next_node;

	ExitNode* exit_node;
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

	/**
	 * - don't reuse previous to not affect decision making
	 */
	double verify_existing_average_score;

	bool is_pass_through;

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
					 AbstractExperiment* parent_experiment,
					 bool skip_explore);
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
								 RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  BranchExperimentHistory* history);
	void explore_backprop(double target_val,
						  RunHelper& run_helper);

	void train_new_activate(AbstractNode*& curr_node,
							std::vector<ContextLayer>& context,
							int& exit_depth,
							AbstractNode*& exit_node,
							RunHelper& run_helper,
							BranchExperimentHistory* history);
	void train_new_target_activate(AbstractNode*& curr_node,
								   std::vector<ContextLayer>& context,
								   int& exit_depth,
								   AbstractNode*& exit_node,
								   RunHelper& run_helper);
	void train_new_backprop(double target_val,
							RunHelper& run_helper);

	bool measure_activate(AbstractNode*& curr_node,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	bool verify_activate(AbstractNode*& curr_node,
						 std::vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper);
	void verify_backprop(double target_val,
						 RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	bool capture_verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	bool root_verify_activate(AbstractNode*& curr_node,
							  std::vector<ContextLayer>& context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  RunHelper& run_helper);

	bool experiment_activate(AbstractNode*& curr_node,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 BranchExperimentHistory* history);
	void experiment_backprop(double target_val,
							 RunHelper& run_helper);

	void experiment_verify_existing_backprop(double target_val,
											 RunHelper& run_helper);

	bool experiment_verify_activate(AbstractNode*& curr_node,
									std::vector<ContextLayer>& context,
									RunHelper& run_helper);
	void experiment_verify_backprop(double target_val,
									RunHelper& run_helper);

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;
	double existing_predicted_score;

	BranchExperimentHistory(BranchExperiment* experiment);
	~BranchExperimentHistory();
};

#endif /* BRANCH_EXPERIMENT_H */