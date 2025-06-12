#ifndef COMMIT_EXPERIMENT_H
#define COMMIT_EXPERIMENT_H

#include <utility>
#include <vector>

#include "abstract_experiment.h"
#include "input.h"

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

class CommitExperimentHistory;
class CommitExperimentState;
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
	std::vector<std::string> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<std::string> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	int step_iter;
	int save_iter;

	double save_sum_score;
	std::vector<int> save_step_types;
	std::vector<std::string> save_actions;
	std::vector<Scope*> save_scopes;
	AbstractNode* save_exit_next_node;
	bool save_is_init;

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

	CommitExperiment(Scope* scope_context,
					 AbstractNode* node_context,
					 bool is_branch);
	~CommitExperiment();
	void decrement(AbstractNode* experiment_node);

	void check_activate(AbstractNode* experiment_node,
						bool is_branch,
						SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 std::string& action,
						 bool& is_next,
						 bool& fetch_action,
						 SolutionWrapper* wrapper);
	void set_action(std::string action,
					SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);
	void backprop(double target_val,
				  SolutionWrapper* wrapper);

	void existing_gather_check_activate(SolutionWrapper* wrapper);
	void existing_gather_backprop();

	void train_existing_check_activate(SolutionWrapper* wrapper,
									   CommitExperimentHistory* history);
	void train_existing_backprop(double target_val,
								 CommitExperimentHistory* history);

	void explore_check_activate(SolutionWrapper* wrapper,
								CommitExperimentHistory* history);
	void explore_step(std::vector<double>& obs,
					  std::string& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper,
					  CommitExperimentState* experiment_state);
	void explore_set_action(std::string action,
							CommitExperimentState* experiment_state);
	void explore_exit_step(SolutionWrapper* wrapper,
						   CommitExperimentState* experiment_state);
	void explore_backprop(double target_val,
						  CommitExperimentHistory* history);

	void find_save_check_activate(SolutionWrapper* wrapper);
	void find_save_step(std::vector<double>& obs,
						std::string& action,
						bool& is_next,
						bool& fetch_action,
						SolutionWrapper* wrapper,
						CommitExperimentState* experiment_state);
	void find_save_set_action(std::string action,
							  CommitExperimentState* experiment_state);
	void find_save_exit_step(SolutionWrapper* wrapper,
							 CommitExperimentState* experiment_state);
	void find_save_backprop(double target_val);

	void commit_existing_gather_check_activate(SolutionWrapper* wrapper);
	void commit_existing_gather_step(std::vector<double>& obs,
									 std::string& action,
									 bool& is_next,
									 SolutionWrapper* wrapper,
									 CommitExperimentState* experiment_state);
	void commit_existing_gather_exit_step(SolutionWrapper* wrapper,
										  CommitExperimentState* experiment_state);
	void commit_existing_gather_backprop();

	void commit_train_existing_check_activate(SolutionWrapper* wrapper);
	void commit_train_existing_step(std::vector<double>& obs,
									std::string& action,
									bool& is_next,
									SolutionWrapper* wrapper,
									CommitExperimentState* experiment_state);
	void commit_train_existing_exit_step(SolutionWrapper* wrapper,
										 CommitExperimentState* experiment_state);
	void commit_train_existing_backprop(double target_val,
										CommitExperimentHistory* history);

	void commit_new_gather_check_activate(SolutionWrapper* wrapper);
	void commit_new_gather_step(std::vector<double>& obs,
								std::string& action,
								bool& is_next,
								SolutionWrapper* wrapper,
								CommitExperimentState* experiment_state);
	void commit_new_gather_exit_step(SolutionWrapper* wrapper,
									 CommitExperimentState* experiment_state);
	void commit_new_gather_backprop();

	void commit_train_new_check_activate(SolutionWrapper* wrapper,
										 CommitExperimentHistory* history);
	void commit_train_new_step(std::vector<double>& obs,
							   std::string& action,
							   bool& is_next,
							   SolutionWrapper* wrapper,
							   CommitExperimentState* experiment_state);
	void commit_train_new_exit_step(SolutionWrapper* wrapper,
									CommitExperimentState* experiment_state);
	void commit_train_new_backprop(double target_val,
								   CommitExperimentHistory* history);

	void measure_check_activate(SolutionWrapper* wrapper);
	void measure_step(std::vector<double>& obs,
					  std::string& action,
					  bool& is_next,
					  SolutionWrapper* wrapper,
					  CommitExperimentState* experiment_state);
	void measure_exit_step(SolutionWrapper* wrapper,
						   CommitExperimentState* experiment_state);
	void measure_backprop(double target_val);

	void abort();

	void clean();
	void add(SolutionWrapper* wrapper);
};

class CommitExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;
	std::vector<double> existing_predicted_scores;

	CommitExperimentHistory(CommitExperiment* experiment);
};

class CommitExperimentState : public AbstractExperimentState {
public:
	bool is_save;
	int step_index;

	CommitExperimentState(CommitExperiment* experiment);
	~CommitExperimentState();
};

#endif /* COMMIT_EXPERIMENT_H */