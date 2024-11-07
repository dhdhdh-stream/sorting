/**
 * - assuming result doesn't help BranchExperiment as much
 *   - may be succeeding due to butterfly effect
 *     - impact/reasoning so far away and difficult that no different than noise for analyze
 *       - remember that many problems are not easily analyzeable from a glance
 *         - need algorithms
 * 
 * - but don't constrain eval either
 *   - it may be butterfly effect far away, but local might still give info
 */

#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class BranchNode;
class Network;
class Problem;
class Scope;
class ScopeNode;
class Solution;

const int BRANCH_EXPERIMENT_STATE_EXPLORE = 0;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 1;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 2;
#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 3;
#endif /* MDEBUG */

class BranchExperimentHistory;
class BranchExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;

	double average_instances_per_run;

	int num_instances_until_target;

	int explore_type;

	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_scopes;
	AbstractNode* curr_exit_next_node;
	// temp
	bool curr_is_mimic;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_scopes;
	AbstractNode* best_exit_next_node;
	// temp
	bool best_is_mimic;

	BranchNode* branch_node;
	ActionNode* ending_node;

	bool is_local;
	std::vector<std::pair<std::pair<std::vector<int>,std::vector<int>>,int>> inputs;
	Network* network;

	double combined_score;

	std::vector<std::vector<double>> obs_histories;
	std::vector<ScopeHistory*> scope_histories;
	std::vector<double> target_val_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	BranchExperiment(Scope* scope_context,
					 AbstractNode* node_context,
					 bool is_branch);
	~BranchExperiment();
	void decrement(AbstractNode* experiment_node);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void train_existing_activate(Problem* problem,
								 BranchExperimentHistory* history);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper);

	bool explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history,
						  BranchExperimentHistory* history);
	void explore_backprop(double target_val,
						  RunHelper& run_helper);

	bool train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							std::vector<ContextLayer>& context,
							RunHelper& run_helper,
							ScopeHistory* scope_history,
							BranchExperimentHistory* history);
	void train_new_backprop(double target_val,
							RunHelper& run_helper);

	bool measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history,
						  BranchExperimentHistory* history);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	bool capture_verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 ScopeHistory* scope_history);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;

	BranchExperimentHistory(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */