#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "action.h"
#include "input.h"
#include "run_helper.h"

class Scope;

const int BRANCH_EXPERIMENT_STATE_EXPLORE = 0;
const int BRANCH_EXPERIMENT_STATE_NEW_GATHER = 1;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 2;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 3;
#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 4;
#endif /* MDEBUG */

class BranchExperimentHistory;
class BranchExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	int num_instances_until_target;

	int explore_type;

	std::vector<int> curr_step_types;
	std::vector<Action> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<Action> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	double new_average_score;
	std::vector<Input> new_inputs;

	std::vector<std::pair<int,int>> new_factor_ids;
	std::vector<double> new_factor_weights;

	double select_percentage;

	double combined_score;

	std::vector<std::vector<double>> input_histories;
	std::vector<std::vector<double>> factor_histories;
	std::vector<double> i_target_val_histories;

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

	void result_activate(AbstractNode* experiment_node,
						 bool is_branch,
						 AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper,
						 ScopeHistory* scope_history);
	void activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history,
						  BranchExperimentHistory* history);
	void explore_backprop(double target_val,
						  RunHelper& run_helper,
						  BranchExperimentHistory* history);

	void new_gather_activate(ScopeHistory* scope_history);
	void new_gather_backprop();

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							RunHelper& run_helper,
							ScopeHistory* scope_history,
							BranchExperimentHistory* history);
	void train_new_backprop(double target_val,
							RunHelper& run_helper,
							BranchExperimentHistory* history);

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 RunHelper& run_helper,
								 ScopeHistory* scope_history);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void clean();
	void add();
};

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	BranchExperimentHistory(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */