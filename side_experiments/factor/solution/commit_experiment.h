#ifndef COMMIT_EXPERIMENT_H
#define COMMIT_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

const int COMMIT_EXPERIMENT_STATE_EXISTING_GATHER = 0;
const int COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING = 1;
const int COMMIT_EXPERIMENT_STATE_EXPLORE = 2;

class CommitExperimentHistory;
class CommitExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> existing_inputs;

	double existing_average_score;
	std::vector<std::pair<int,int>> existing_factor_ids;
	std::vector<double> existing_factor_weights;

	int num_instances_until_target;

	int explore_type;

	std::vector<int> curr_step_types;
	std::vector<Action*> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<Action*> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	std::vector<std::vector<double>> input_histories;
	std::vector<std::map<std::pair<int,int>, double>> factor_histories;
	std::vector<double> i_target_val_histories;
	std::vector<double> o_target_val_histories;

	CommitExperiment(Scope* scope_context,
					 AbstractNode* node_context,
					 bool is_branch);
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

	void existing_gather_activate(ScopeHistory* scope_history);
	void existing_gather_backprop();

	void train_existing_activate(RunHelper& run_helper,
								 ScopeHistory* scope_history,
								 CommitExperimentHistory* history);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history,
						  CommitExperimentHistory* history);
	void explore_backprop(double target_val,
						  RunHelper& run_helper);

	void finalize(Solution* duplicate);
};

class CommitExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;
	std::vector<double> existing_predicted_scores;

	CommitExperimentHistory(CommitExperiment* experiment);
};

#endif /* COMMIT_EXPERIMENT_H */