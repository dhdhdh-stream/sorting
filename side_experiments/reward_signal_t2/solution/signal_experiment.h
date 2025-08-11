#ifndef SIGNAL_EXPERIMENT_H
#define SIGNAL_EXPERIMENT_H

#include <vector>

#include "signal.h"

class Network;
class Problem;
class SolutionWrapper;

const int SIGNAL_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int SIGNAL_EXPERIMENT_STATE_FIND_SAFE = 1;
const int SIGNAL_EXPERIMENT_STATE_EXPLORE = 2;
const int SIGNAL_EXPERIMENT_STATE_DONE = 3;

class SignalExperimentHistory;
class SignalExperiment {
public:
	int state;

	std::vector<double> existing_scores;

	std::vector<int> pre_actions;
	std::vector<int> post_actions;

	std::vector<double> new_scores;

	std::vector<std::vector<std::vector<double>>> pre_obs_histories;
	std::vector<std::vector<std::vector<double>>> post_obs_histories;
	std::vector<double> target_val_histories;

	std::vector<Signal*> signals;
	double miss_average_guess;

	SignalExperiment();

	void pre_activate(Problem* problem,
					  SignalExperimentHistory* history);
	void post_activate(Problem* problem,
					   SignalExperimentHistory* history);
	void backprop(double target_val,
				  SignalExperimentHistory* history);

	void find_safe_pre_activate(Problem* problem);
	void find_safe_post_activate(Problem* problem);
	void find_safe_backprop(double target_val);

	void explore_pre_activate(Problem* problem,
							  SignalExperimentHistory* history);
	void explore_post_activate(Problem* problem,
							   SignalExperimentHistory* history);
	void explore_backprop(double target_val,
						  SignalExperimentHistory* history);

	void add(SolutionWrapper* wrapper);

private:
	bool split_helper(std::vector<std::vector<std::vector<double>>>& pre_obs_histories,
					  std::vector<std::vector<std::vector<double>>>& post_obs_histories,
					  std::vector<bool>& new_match_input_is_pre,
					  std::vector<int>& new_match_input_indexes,
					  std::vector<int>& new_match_input_obs_indexes,
					  Network*& new_match_network);
	bool train_score(std::vector<std::vector<std::vector<double>>>& pre_obs_histories,
					 std::vector<std::vector<std::vector<double>>>& post_obs_histories,
					 std::vector<double>& target_val_histories,
					 std::vector<bool>& new_score_input_is_pre,
					 std::vector<int>& new_score_input_indexes,
					 std::vector<int>& new_score_input_obs_indexes,
					 Network*& new_score_network);
	void create_reward_signal_helper(std::vector<std::vector<std::vector<double>>>& pre_obs_histories,
									 std::vector<std::vector<std::vector<double>>>& post_obs_histories,
									 std::vector<double>& target_val_histories,
									 std::vector<Signal*>& signals,
									 double& miss_average_guess);
};

class SignalExperimentHistory {
public:
	std::vector<std::vector<double>> pre_obs;
	std::vector<std::vector<double>> post_obs;
};

#endif /* SIGNAL_EXPERIMENT_H */