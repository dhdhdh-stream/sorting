#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "action.h"
#include "input.h"
#include "run_helper.h"

class Scope;

const int BRANCH_EXPERIMENT_STATE_EXISTING_GATHER = 0;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 1;
const int BRANCH_EXPERIMENT_STATE_NEW_GATHER = 2;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 3;
const int BRANCH_EXPERIMENT_STATE_MEASURE_1_PERCENT = 4;
const int BRANCH_EXPERIMENT_STATE_MEASURE_5_PERCENT = 5;
const int BRANCH_EXPERIMENT_STATE_MEASURE_10_PERCENT = 6;
const int BRANCH_EXPERIMENT_STATE_MEASURE_25_PERCENT = 7;
const int BRANCH_EXPERIMENT_STATE_MEASURE_50_PERCENT = 8;

class BranchExperimentHistory;
class BranchExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	int sum_num_instances;

	double average_instances_per_run;
	int num_instances_until_target;

	std::vector<Input> existing_inputs;
	std::vector<std::pair<int,int>> existing_factor_ids;

	std::vector<std::vector<double>> existing_input_histories;
	std::vector<std::vector<double>> existing_factor_histories;
	std::vector<double> existing_target_val_histories;

	std::vector<std::vector<double>> explore_input_histories;
	std::vector<std::vector<double>> explore_factor_histories;
	std::vector<std::vector<int>> explore_step_types;
	std::vector<std::vector<Action>> explore_actions;
	std::vector<std::vector<Scope*>> explore_scopes;
	std::vector<AbstractNode*> explore_exit_next_nodes;
	std::vector<double> explore_target_val_histories;

	double existing_average_score;
	std::vector<double> existing_factor_weights;

	std::vector<int> best_step_types;
	std::vector<Action> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	std::vector<Input> new_inputs;
	std::vector<std::pair<int,int>> new_factor_ids;

	std::vector<std::vector<double>> new_input_histories;
	std::vector<std::vector<double>> new_factor_histories;
	std::vector<double> new_target_val_histories;

	double new_average_score;
	std::vector<double> new_factor_weights;

	double select_percentage;

	double existing_sum_score;
	int existing_count;
	double combined_sum_score;
	int combined_count;

	BranchExperiment(Scope* scope_context,
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
								 BranchExperimentHistory* history);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper,
								 BranchExperimentHistory* history);

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
						  ScopeHistory* scope_history,
						  BranchExperimentHistory* history);
	void measure_backprop(double target_val,
						  RunHelper& run_helper,
						  BranchExperimentHistory* history);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_factor(Scope* scope,
						int original_node_id,
						int original_factor_index,
						int new_node_id,
						int new_factor_index);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);
	void replace_scope(Scope* original_scope,
					   Scope* new_scope,
					   int new_scope_node_id);

	void clean();
	void add();
};

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<double> existing_predicted_scores;

	BranchExperimentHistory(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */