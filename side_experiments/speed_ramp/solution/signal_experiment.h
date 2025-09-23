#ifndef SIGNAL_EXPERIMENT_H
#define SIGNAL_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class DefaultSignal;
class Signal;
class SignalNetwork;
class SolutionWrapper;

const int SIGNAL_EXPERIMENT_STATE_INITIAL_C1 = 0;
const int SIGNAL_EXPERIMENT_STATE_INITIAL_C2 = 1;
const int SIGNAL_EXPERIMENT_STATE_INITIAL_C3 = 2;
const int SIGNAL_EXPERIMENT_STATE_INITIAL_C4 = 3;
const int SIGNAL_EXPERIMENT_STATE_RAMP = 4;
const int SIGNAL_EXPERIMENT_STATE_GATHER = 5;
const int SIGNAL_EXPERIMENT_STATE_WRAPUP = 6;
/**
 * - if new better, add to scope, and swap existing and new
 */

#if defined(MDEBUG) && MDEBUG
const int EXISTING_CURRENT_SAMPLES = 80;
const int EXISTING_EXPLORE_SAMPLES = 40;

const int NEW_CURRENT_SAMPLES = 80;
const int NEW_EXPLORE_SAMPLES = 40;
#else
const int EXISTING_CURRENT_SAMPLES = 8000;
const int EXISTING_EXPLORE_SAMPLES = 4000;

const int NEW_CURRENT_SAMPLES = 8000;
const int NEW_EXPLORE_SAMPLES = 4000;
#endif /* MDEBUG */

class SignalExperimentHistory;
class SignalExperiment : public AbstractExperiment {
public:
	Scope* scope_context;

	int state;
	int state_iter;
	int num_fail;

	std::vector<bool> pre_action_initialized;
	std::vector<int> pre_actions;
	std::vector<bool> post_action_initialized;
	std::vector<int> post_actions;

	std::vector<Signal*> adjusted_previous_signals;

	int curr_ramp;

	std::vector<double> existing_scores;
	std::vector<double> new_scores;

	/**
	 * - over time, should naturally include current, diversity, and traps
	 */
	std::vector<std::vector<std::vector<double>>> existing_current_pre_obs;
	std::vector<std::vector<std::vector<double>>> existing_current_post_obs;
	std::vector<double> existing_current_scores;

	std::vector<std::vector<std::vector<double>>> existing_explore_pre_obs;
	std::vector<std::vector<std::vector<double>>> existing_explore_post_obs;
	std::vector<double> existing_explore_scores;

	std::vector<std::vector<std::vector<double>>> new_current_pre_obs;
	std::vector<std::vector<std::vector<double>>> new_current_post_obs;
	std::vector<double> new_current_scores;

	std::vector<std::vector<std::vector<double>>> new_explore_pre_obs;
	std::vector<std::vector<std::vector<double>>> new_explore_post_obs;
	std::vector<double> new_explore_scores;

	SignalExperiment(Scope* scope_context);
	~SignalExperiment();

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
				  SignalExperimentHistory* history,
				  SolutionWrapper* wrapper);

	void initial_backprop(double target_val,
						  SignalExperimentHistory* history,
						  SolutionWrapper* wrapper);

	void ramp_backprop(double target_val,
					   SignalExperimentHistory* history,
					   SolutionWrapper* wrapper);

	void gather_backprop(double target_val,
						 SignalExperimentHistory* history,
						 SolutionWrapper* wrapper);

	void wrapup_backprop();

	void set_actions();
	bool split_helper(std::vector<std::vector<std::vector<double>>>& current_pre_obs,
					  std::vector<std::vector<std::vector<double>>>& current_post_obs,
					  std::vector<std::vector<std::vector<double>>>& explore_pre_obs,
					  std::vector<std::vector<std::vector<double>>>& explore_post_obs,
					  std::vector<bool>& new_match_input_is_pre,
					  std::vector<int>& new_match_input_indexes,
					  std::vector<int>& new_match_input_obs_indexes,
					  SignalNetwork*& new_match_network);
	void train_score(std::vector<std::vector<std::vector<double>>>& pre_obs_histories,
					 std::vector<std::vector<std::vector<double>>>& post_obs_histories,
					 std::vector<double>& target_val_histories,
					 std::vector<bool>& new_score_input_is_pre,
					 std::vector<int>& new_score_input_indexes,
					 std::vector<int>& new_score_input_obs_indexes,
					 SignalNetwork*& new_score_network);
	DefaultSignal* train_existing_default();
	DefaultSignal* train_new_default();
	void create_reward_signal_helper(std::vector<std::vector<std::vector<double>>>& current_pre_obs,
									 std::vector<std::vector<std::vector<double>>>& current_post_obs,
									 std::vector<double>& current_scores,
									 std::vector<std::vector<std::vector<double>>>& explore_pre_obs,
									 std::vector<std::vector<std::vector<double>>>& explore_post_obs,
									 std::vector<double>& explore_scores,
									 DefaultSignal* default_signal,
									 std::vector<Signal*>& previous_signals,
									 std::vector<Signal*>& signals,
									 double& misguess_average);
};

class SignalExperimentHistory {
public:
	bool is_on;

	std::vector<std::vector<std::vector<double>>> pre_obs;
	std::vector<std::vector<std::vector<double>>> post_obs;
	std::vector<bool> inner_is_explore;

	std::vector<bool> signal_is_set;
	std::vector<double> signal_vals;
	std::vector<bool> outer_is_explore;

	SignalExperimentHistory(SignalExperiment* experiment);
};

class SignalExperimentState : public AbstractExperimentState {
public:
	bool is_pre;
	int index;

	SignalExperimentState(SignalExperiment* experiment);
};

#endif /* SIGNAL_EXPERIMENT_H */