#ifndef SIGNAL_EXPERIMENT_H
#define SIGNAL_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class Explore;
class Problem;
class Scope;
class ScopeHistory;
class SignalInstance;
class SignalNetwork;
class SolutionWrapper;

const int SIGNAL_EXPERIMENT_STATE_FIND_SAFE = 0;
const int SIGNAL_EXPERIMENT_STATE_EXPLORE = 1;
const int SIGNAL_EXPERIMENT_STATE_DONE = 2;

const int SIGNAL_EXPERIMENT_STATE_VERIFY = 3;

class SignalExperimentHistory;
class SignalExperiment : public AbstractExperiment {
public:
	int scope_context_id;

	int state;
	int state_iter;
	int solution_index;

	std::vector<int> pre_actions;
	std::vector<int> post_actions;

	std::vector<std::vector<double>> new_scores;

	std::vector<std::vector<std::vector<double>>> existing_pre_obs;
	std::vector<std::vector<std::vector<double>>> existing_post_obs;
	std::vector<double> existing_scores;

	double average_instances_per_run;
	int num_instances_until_target;

	AbstractNode* explore_node;
	bool explore_is_branch;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	std::vector<std::vector<std::vector<double>>> explore_pre_obs;
	std::vector<std::vector<std::vector<double>>> explore_post_obs;
	std::vector<double> explore_scores;

	std::vector<SignalInstance*> signals;
	double miss_average_guess;

	SignalExperiment(int scope_context_id,
					 SolutionWrapper* wrapper);
	~SignalExperiment();

	bool check_signal(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper);
	void check_activate(AbstractNode* experiment_node,
						bool is_branch,
						SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 bool& fetch_action,
						 SolutionWrapper* wrapper);
	void set_action(int action,
					SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);
	void backprop(double target_val,
				  SolutionWrapper* wrapper);

	bool find_safe_check_signal(std::vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper);
	void find_safe_backprop(double target_val,
							SolutionWrapper* wrapper);

	bool explore_check_signal(std::vector<double>& obs,
							  int& action,
							  bool& is_next,
							  SolutionWrapper* wrapper);
	void explore_check_activate(AbstractNode* experiment_node,
								bool is_branch,
								SolutionWrapper* wrapper);
	void explore_experiment_step(std::vector<double>& obs,
								 int& action,
								 bool& is_next,
								 bool& fetch_action,
								 SolutionWrapper* wrapper);
	void explore_set_action(int action,
							SolutionWrapper* wrapper);
	void explore_experiment_exit_step(SolutionWrapper* wrapper);
	void explore_backprop(double target_val,
						  SolutionWrapper* wrapper);

	bool verify_check_signal(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper);
	void verify_check_activate(AbstractNode* experiment_node,
							   bool is_branch,
							   SolutionWrapper* wrapper);
	void verify_experiment_step(std::vector<double>& obs,
								int& action,
								bool& is_next,
								bool& fetch_action,
								SolutionWrapper* wrapper);
	void verify_set_action(int action,
						   SolutionWrapper* wrapper);
	void verify_experiment_exit_step(SolutionWrapper* wrapper);
	void verify_backprop(double target_val,
						 SolutionWrapper* wrapper);

private:
	void set_actions(SolutionWrapper* wrapper);
	void set_explore(SolutionWrapper* wrapper);
	bool split_helper(std::vector<bool>& new_match_input_is_pre,
					  std::vector<int>& new_match_input_indexes,
					  std::vector<int>& new_match_input_obs_indexes,
					  SignalNetwork*& new_match_network);
	void train_score(std::vector<std::vector<std::vector<double>>>& positive_pre_obs_histories,
					 std::vector<std::vector<std::vector<double>>>& positive_post_obs_histories,
					 std::vector<double>& positive_target_val_histories,
					 std::vector<std::vector<std::vector<double>>>& pre_obs_histories,
					 std::vector<std::vector<std::vector<double>>>& post_obs_histories,
					 std::vector<double>& target_val_histories,
					 std::vector<bool>& new_score_input_is_pre,
					 std::vector<int>& new_score_input_indexes,
					 std::vector<int>& new_score_input_obs_indexes,
					 SignalNetwork*& new_score_network);
	bool create_reward_signal_helper(SolutionWrapper* wrapper);
};

class SignalExperimentInstanceHistory {
public:
	ScopeHistory* scope_history;
	ScopeHistory* signal_needed_from;

	SignalExperimentInstanceHistory();
};

class SignalExperimentState : public AbstractExperimentState {
public:
	int step_index;

	SignalExperimentState(SignalExperiment* experiment);
};

#endif /* SIGNAL_EXPERIMENT_H */