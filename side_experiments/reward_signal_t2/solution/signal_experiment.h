/**
 * - for now, only worry about single explore between pre and post
 * TODO: support multi-experiment
 */

#ifndef SIGNAL_EXPERIMENT_H
#define SIGNAL_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "signal.h"

class Explore;
class Problem;
class Scope;
class ScopeHistory;
class SignalNetwork;
class SolutionWrapper;

const int SIGNAL_EXPERIMENT_STATE_FIND_SAFE = 0;
const int SIGNAL_EXPERIMENT_STATE_EXPLORE = 1;
const int SIGNAL_EXPERIMENT_STATE_DONE = 2;

class SignalExperimentHistory;
class SignalExperiment : public AbstractExperiment {
public:
	Scope* scope_context;

	int state;
	int state_iter;

	double existing_average;
	double existing_standard_deviation;

	std::vector<int> pre_actions;
	std::vector<int> post_actions;

	std::vector<double> new_scores;

	double existing_average_outer_signal;

	int num_instances_until_target;

	Explore* curr_explore;

	/**
	 * - to help specifically recognize positive situations
	 */
	std::vector<std::vector<std::vector<double>>> positive_pre_obs_histories;
	std::vector<std::vector<std::vector<double>>> positive_post_obs_histories;
	std::vector<double> positive_target_val_histories;
	std::vector<Explore*> positive_explores;
	/**
	 * TODO:
	 * - re-utilize good explores
	 *   - though have to solve measure not being accurate due to not accounting for context
	 */

	std::vector<std::vector<std::vector<double>>> pre_obs_histories;
	std::vector<std::vector<std::vector<double>>> post_obs_histories;
	std::vector<double> target_val_histories;

	std::vector<Signal*> signals;
	double miss_average_guess;

	std::vector<ScopeHistory*> new_scope_histories;
	std::vector<double> new_target_val_histories;

	SignalExperiment(Scope* scope_context,
					 SolutionWrapper* wrapper);
	~SignalExperiment();

	bool check_signal(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper);
	void backprop(double target_val,
				  SolutionWrapper* wrapper);

	void find_safe_backprop(double target_val);

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
	void explore_backprop(double target_val,
						  SolutionWrapper* wrapper);

private:
	void set_actions();
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
	void create_reward_signal_helper(SolutionWrapper* wrapper);
};

class SignalExperimentHistory {
public:
	bool is_hit;

	ScopeHistory* scope_history;

	ScopeHistory* signal_needed_from;

	SignalExperimentHistory();
};

class SignalExperimentState : public AbstractExperimentState {
public:
	int step_index;

	SignalExperimentState(SignalExperiment* experiment);
};

#endif /* SIGNAL_EXPERIMENT_H */