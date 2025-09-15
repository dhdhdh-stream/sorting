// TODO: on failure, wind actions back down?

// - actually, how to schedule when signals still needed for inner?
//   - don't activate all the time
//   - use outer when needed

#ifndef SIGNAL_EVAL_EXPERIMENT_H
#define SIGNAL_EVAL_EXPERIMENT_H

#include <vector>

class DefaultSignal;
class Scope;
class Signal;
class SignalNetwork;
class SolutionWrapper;

const int SIGNAL_EVAL_EXPERIMENT_STATE_GATHER = 0;
const int SIGNAL_EVAL_EXPERIMENT_STATE_DONE = 1;

class SignalEvalExperimentHistory;
class SignalEvalExperiment {
public:
	Scope* scope_context;

	int state;

	std::vector<int> pre_actions;
	std::vector<int> post_actions;

	std::vector<Signal*> previous_signals;

	/**
	 * - over time, should naturally include current, diversity, and traps
	 */
	std::vector<std::vector<std::vector<double>>> pre_obs;
	std::vector<std::vector<std::vector<double>>> post_obs;
	std::vector<double> scores;

	std::vector<std::vector<std::vector<double>>> explore_pre_obs;
	std::vector<std::vector<std::vector<double>>> explore_post_obs;
	std::vector<double> explore_scores;

	std::vector<Signal*> signals;
	DefaultSignal* default_signal;

	double misguess_average;

	SignalEvalExperiment();
	~SignalEvalExperiment();

	bool check_signal_activate(std::vector<double>& obs,
							   int& action,
							   bool& is_next,
							   SolutionWrapper* wrapper,
							   SignalEvalExperimentHistory* history);
	void backprop(double target_val,
				  SignalEvalExperimentHistory* history,
				  SolutionWrapper* wrapper);

	void add(SolutionWrapper* wrapper);

private:
	bool split_helper(std::vector<bool>& new_match_input_is_pre,
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
	void create_reward_signal_helper();
};

class SignalEvalExperimentHistory {
public:
	bool is_on;

	std::vector<std::vector<std::vector<double>>> pre_obs;
	std::vector<std::vector<std::vector<double>>> post_obs;
	std::vector<bool> inner_is_explore;

	std::vector<bool> signal_is_set;
	std::vector<double> signal_vals;
	std::vector<bool> outer_is_explore;

	SignalEvalExperimentHistory();
};

#endif /* SIGNAL_EVAL_EXPERIMENT_H */