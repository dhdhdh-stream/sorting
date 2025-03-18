#ifndef MULTI_BRANCH_EXPERIMENT_H
#define MULTI_BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "action.h"
#include "run_helper.h"

class Scope;

const int MULTI_BRANCH_EXPERIMENT_STATE_EXISTING_GATHER = 0;
const int MULTI_BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 1;
const int MULTI_BRANCH_EXPERIMENT_STATE_EXPLORE = 2;
const int MULTI_BRANCH_EXPERIMENT_STATE_NEW_GATHER = 3;
const int MULTI_BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 4;
const int MULTI_BRANCH_EXPERIMENT_STATE_MEASURE = 5;

class MultiBranchExperimentHistory;
class MultiBranchExperiment : public AbstractExperiment {
public:
	int id;

	int state;
	int state_iter;

	int sum_num_instances;

	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> existing_inputs;
	std::vector<std::pair<int,int>> existing_factor_ids;

	double existing_average_score;
	std::vector<double> existing_factor_weights;

	double average_instances_per_run;
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
	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> new_inputs;

	std::vector<std::pair<int,int>> new_factor_ids;
	std::vector<double> new_factor_weights;

	double select_percentage;

	std::vector<double> existing_target_vals;
	std::vector<std::vector<int>> existing_influence_indexes;
	std::vector<double> new_target_vals;
	std::vector<std::vector<int>> new_influence_indexes;
	std::map<int, int> influence_mapping;

	double improvement;

	std::vector<std::vector<double>> input_histories;
	std::vector<std::vector<double>> factor_histories;
	std::vector<double> i_target_val_histories;

	MultiBranchExperiment(Scope* scope_context,
						  AbstractNode* node_context,
						  bool is_branch);
	void decrement(AbstractNode* experiment_node);

	void activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void existing_gather_activate(ScopeHistory* scope_history);
	void existing_gather_backprop();

	void train_existing_activate(RunHelper& run_helper,
								 ScopeHistory* scope_history,
								 MultiBranchExperimentHistory* history);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper,
								 MultiBranchExperimentHistory* history);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history,
						  MultiBranchExperimentHistory* history);
	void explore_backprop(double target_val,
						  RunHelper& run_helper,
						  MultiBranchExperimentHistory* history);

	void new_gather_activate(ScopeHistory* scope_history);
	void new_gather_backprop();

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							RunHelper& run_helper,
							ScopeHistory* scope_history,
							MultiBranchExperimentHistory* history);
	void train_new_backprop(double target_val,
							RunHelper& run_helper,
							MultiBranchExperimentHistory* history);

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history,
						  MultiBranchExperimentHistory* history);
	void measure_backprop(double target_val,
						  RunHelper& run_helper,
						  MultiBranchExperimentHistory* history);

	void clear();
	void finalize(Solution* duplicate);
};

class MultiBranchExperimentHistory : public AbstractExperimentHistory {
public:
	bool is_active;

	int instance_count;
	std::vector<double> existing_predicted_scores;

	MultiBranchExperimentHistory(MultiBranchExperiment* experiment);
};

#endif /* MULTI_BRANCH_EXPERIMENT_H */