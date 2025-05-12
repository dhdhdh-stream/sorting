#ifndef COMMIT_EXPERIMENT_H
#define COMMIT_EXPERIMENT_H

#include <utility>
#include <vector>

#include "abstract_experiment.h"
#include "action.h"
#include "input.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;

const int COMMIT_EXPERIMENT_STATE_EXISTING_GATHER = 0;
const int COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING = 1;
const int COMMIT_EXPERIMENT_STATE_EXPLORE = 2;
const int COMMIT_EXPERIMENT_STATE_FIND_SAVE = 3;
const int COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER = 4;
const int COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING = 5;
const int COMMIT_EXPERIMENT_STATE_COMMIT_NEW_GATHER = 6;
const int COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW = 7;
const int COMMIT_EXPERIMENT_STATE_MEASURE = 8;
#if defined(MDEBUG) && MDEBUG
const int COMMIT_EXPERIMENT_STATE_CAPTURE_VERIFY = 9;
#endif /* MDEBUG */

class CommitExperimentHistory;
class CommitExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	int sum_num_instances;

	double o_existing_average_score;

	std::vector<Input> existing_inputs;
	std::vector<std::pair<int,int>> existing_factor_ids;

	double existing_average_score;
	std::vector<double> existing_factor_weights;

	double average_instances_per_run;
	int num_instances_until_target;

	std::vector<int> curr_step_types;
	std::vector<Action> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<Action> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	int step_iter;
	int save_iter;

	double save_sum_score;
	std::vector<int> save_step_types;
	std::vector<Action> save_actions;
	std::vector<Scope*> save_scopes;
	AbstractNode* save_exit_next_node;

	std::vector<AbstractNode*> new_nodes;

	std::vector<Input> commit_existing_inputs;
	std::vector<std::pair<int,int>> commit_existing_factor_ids;

	double commit_existing_average_score;
	std::vector<double> commit_existing_factor_weights;

	double commit_new_average_score;
	std::vector<Input> commit_new_inputs;

	std::vector<std::pair<int,int>> commit_new_factor_ids;
	std::vector<double> commit_new_factor_weights;

	double combined_score;

	std::vector<std::vector<double>> input_histories;
	std::vector<std::vector<double>> factor_histories;
	std::vector<double> i_target_val_histories;
	std::vector<double> o_target_val_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	CommitExperiment(Scope* scope_context,
					 AbstractNode* node_context,
					 bool is_branch);
	~CommitExperiment();
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
								 CommitExperimentHistory* history);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper,
								 CommitExperimentHistory* history);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history,
						  CommitExperimentHistory* history);
	void explore_backprop(double target_val,
						  RunHelper& run_helper,
						  CommitExperimentHistory* history);

	void find_save_activate(AbstractNode*& curr_node,
							Problem* problem,
							RunHelper& run_helper);
	void find_save_backprop(double target_val,
							RunHelper& run_helper);

	void commit_existing_gather_activate(AbstractNode*& curr_node,
										 Problem* problem,
										 RunHelper& run_helper,
										 ScopeHistory* scope_history);
	void commit_existing_gather_backprop();

	void commit_train_existing_activate(AbstractNode*& curr_node,
										Problem* problem,
										RunHelper& run_helper,
										ScopeHistory* scope_history,
										CommitExperimentHistory* history);
	void commit_train_existing_backprop(double target_val,
										RunHelper& run_helper,
										CommitExperimentHistory* history);

	void commit_new_gather_activate(AbstractNode*& curr_node,
									Problem* problem,
									RunHelper& run_helper,
									ScopeHistory* scope_history);
	void commit_new_gather_backprop();

	void commit_train_new_activate(AbstractNode*& curr_node,
								   Problem* problem,
								   RunHelper& run_helper,
								   ScopeHistory* scope_history,
								   CommitExperimentHistory* history);
	void commit_train_new_backprop(double target_val,
								   RunHelper& run_helper,
								   CommitExperimentHistory* history);

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

class CommitExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;
	std::vector<double> existing_predicted_scores;

	CommitExperimentHistory(CommitExperiment* experiment);
};

#endif /* COMMIT_EXPERIMENT_H */