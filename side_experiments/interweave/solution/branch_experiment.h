#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <utility>
#include <vector>

#include "abstract_experiment.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;

const int BRANCH_EXPERIMENT_STATE_EXISTING_GATHER = 0;
const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 1;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 2;
const int BRANCH_EXPERIMENT_STATE_NEW_GATHER = 3;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 4;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 5;

const int BRANCH_EXPERIMENT_NUM_CONCURRENT = 10;
const int BRANCH_EXPERIMENT_NUM_CONCURRENT_PER_RUN = 3;

class BranchExperimentOverallHistory;
class BranchExperimentInstanceHistory;
class BranchExperiment : public AbstractExperiment {
public:
	int state;
	int instance_iter;
	int run_iter;

	int sum_num_instances;

	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> existing_inputs;
	std::vector<std::pair<int,int>> existing_factor_ids;

	double existing_average_score;
	std::vector<double> existing_factor_weights;

	double average_instances_per_run;
	int num_instances_until_target;

	std::vector<double> surprises;
	std::vector<std::vector<int>> step_types;
	std::vector<std::vector<Action>> actions;
	std::vector<std::vector<Scope*>> scopes;
	std::vector<AbstractNode*> exit_next_node;

	std::vector<std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>>> new_inputs;
	std::vector<std::vector<std::pair<int,int>>> new_factor_ids;

	std::vector<double> new_average_score;
	std::vector<std::vector<double>> new_factor_weights;

	std::vector<double> select_percentage;

	std::vector<double> combined_scores;
	std::vector<int> combined_counts;

	int best_concurrent_index;

	std::vector<std::vector<double>> existing_input_histories;
	std::vector<std::vector<double>> existing_factor_histories;
	std::vector<double> existing_target_val_histories;
	std::vector<std::vector<std::vector<double>>> new_input_histories;
	std::vector<std::vector<std::vector<double>>> new_factor_histories;
	std::vector<std::vector<double>> new_target_val_histories;

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
	void backprop(AbstractExperimentInstanceHistory* instance_history,
				  double target_val);
	void update(AbstractExperimentOverallHistory* overall_history,
				double target_val);

	void existing_gather_activate(ScopeHistory* scope_history);
	void existing_gather_update();

	void train_existing_activate(ScopeHistory* scope_history,
								 BranchExperimentOverallHistory* overall_history);
	void train_existing_update(BranchExperimentOverallHistory* overall_history,
							   double target_val);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history,
						  BranchExperimentOverallHistory* overall_history);
	void explore_backprop(BranchExperimentInstanceHistory* instance_history,
						  double target_val);
	void explore_update();

	void new_gather_activate(ScopeHistory* scope_history,
							 BranchExperimentOverallHistory* overall_history);
	void new_gather_update();

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							RunHelper& run_helper,
							ScopeHistory* scope_history,
							BranchExperimentOverallHistory* overall_history);
	void train_new_backprop(BranchExperimentInstanceHistory* instance_history,
							double target_val);
	void train_new_update();

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history,
						  BranchExperimentOverallHistory* overall_history);
	void measure_backprop(BranchExperimentInstanceHistory* instance_history,
						  double target_val);
	void measure_update();

	void finalize();
};

class BranchExperimentOverallHistory : public AbstractExperimentOverallHistory {
public:
	int instance_count;

	bool has_target;

	std::vector<int> active_concurrents;

	BranchExperimentOverallHistory(BranchExperiment* experiment);
};

class BranchExperimentInstanceHistory : public AbstractExperimentInstanceHistory {
public:
	double existing_predicted_score;

	std::vector<int> curr_step_types;
	std::vector<Action> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	int concurrent_index;

	BranchExperimentInstanceHistory(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */