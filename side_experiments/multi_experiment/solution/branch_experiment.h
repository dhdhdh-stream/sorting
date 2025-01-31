#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "action.h"
#include "run_helper.h"

class Scope;

const int BRANCH_EXPERIMENT_STATE_EXISTING_GATHER = 0;
const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 1;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 2;
const int BRANCH_EXPERIMENT_STATE_NEW_GATHER = 3;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 4;

class BranchExperimentHistory;
class BranchExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> existing_inputs;
	std::vector<std::pair<int,int>> existing_factor_ids;

	double existing_average_score;
	std::vector<double> existing_factor_weights;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<Action> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> new_inputs;
	std::vector<std::pair<int,int>> new_factor_ids;

	double new_average_score;
	std::vector<double> new_factor_weights;

	double select_percentage;

	std::vector<std::vector<double>> input_histories;
	std::vector<std::vector<double>> factor_histories;
	std::vector<double> i_target_val_histories;

	BranchExperiment(Scope* scope_context,
					 AbstractNode* node_context,
					 bool is_branch);
	void decrement(AbstractNode* experiment_node);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void backprop(AbstractExperimentHistory* history);
	void update();

	void existing_gather_activate(ScopeHistory* scope_history);
	void existing_gather_backprop();
	void existing_gather_update();

	void train_existing_activate(ScopeHistory* scope_history);
	void train_existing_backprop(BranchExperimentHistory* history);
	void train_existing_update();

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history,
						  BranchExperimentHistory* history);
	void explore_backprop(BranchExperimentHistory* history);
	void explore_update();

	void new_gather_activate(ScopeHistory* scope_history);
	void new_gather_backprop();
	void new_gather_update();

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							RunHelper& run_helper,
							ScopeHistory* scope_history,
							BranchExperimentHistory* history);
	void train_new_backprop(BranchExperimentHistory* history);
	void train_new_update();

	void finalize();
};

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	double existing_predicted_score;

	std::vector<int> curr_step_types;
	std::vector<Action> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	BranchExperimentHistory(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */