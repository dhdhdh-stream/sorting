// TODO: need to chain BranchExperiments as well
// - recognize when a sequence is safe to perform, even though doesn't direclty lead to improvement
//   - but then followup makes things good

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
class PassThroughExperiment;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
/**
 * - select first that is significant improvement
 *   - don't select "best" as might not have been learned for actual best
 *     - so may select lottery instead of actual best
 * 
 * TODO: select multiple per TRAIN_EXISTING
 */
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

class BranchExperimentHistory;
class BranchExperiment : public AbstractExperiment {
public:
	bool skip_explore;

	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;
	int input_max_depth;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_average_misguess;
	double existing_misguess_standard_deviation;

	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<ScopeNode*> existing_scopes;
	std::vector<ScopeNode*> potential_scopes;
	std::vector<std::set<int>> catch_throw_ids;
	int exit_depth;
	AbstractNode* exit_next_node;
	int exit_throw_id;

	BranchNode* branch_node;
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
					 PassThroughExperiment* parent_experiment,
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
						  RunHelper& run_helper,
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