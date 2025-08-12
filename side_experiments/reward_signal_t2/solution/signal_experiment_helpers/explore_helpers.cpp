#include "signal_experiment.h"

#include "globals.h"
#include "problem.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_POSITIVE = 10;
const int MIN_NUM_EXPLORE = 40;
const int MAX_NUM_EXPLORE = 80;
#else
const int NUM_POSITIVE = 1000;
const int MIN_NUM_EXPLORE = 4000;
const int MAX_NUM_EXPLORE = 8000;
#endif /* MDEBUG */

void SignalExperiment::explore_pre_activate(
		Problem* problem,
		SignalExperimentHistory* history) {
	history->pre_obs.push_back(problem->get_observations());
	for (int a_index = 0; a_index < (int)this->pre_actions.size(); a_index++) {
		problem->perform_action(this->pre_actions[a_index]);

		history->pre_obs.push_back(problem->get_observations());
	}

	geometric_distribution<int> num_random_distribution(0.05);
	uniform_int_distribution<int> action_distribution(0, 2);
	int num_random = num_random_distribution(generator);
	for (int a_index = 0; a_index < num_random; a_index++) {
		problem->perform_action(action_distribution(generator));
	}
}

void SignalExperiment::explore_post_activate(
		Problem* problem,
		SignalExperimentHistory* history) {
	history->post_obs.push_back(problem->get_observations());
	for (int a_index = 0; a_index < (int)this->post_actions.size(); a_index++) {
		problem->perform_action(this->post_actions[a_index]);

		history->post_obs.push_back(problem->get_observations());
	}
}

void SignalExperiment::explore_backprop(
		double target_val,
		SignalExperimentHistory* history) {
	if (target_val > this->existing_average_score) {
		this->positive_pre_obs_histories.push_back(history->pre_obs);
		this->positive_post_obs_histories.push_back(history->post_obs);
		this->positive_target_val_histories.push_back(target_val);
	}

	if (this->pre_obs_histories.size() < MAX_NUM_EXPLORE) {
		this->pre_obs_histories.push_back(history->pre_obs);
		this->post_obs_histories.push_back(history->post_obs);
		this->target_val_histories.push_back(target_val);
	}

	if (this->positive_pre_obs_histories.size() >= NUM_POSITIVE
			&& this->pre_obs_histories.size() >= MIN_NUM_EXPLORE) {
		create_reward_signal_helper(this->positive_pre_obs_histories,
									this->positive_post_obs_histories,
									this->positive_target_val_histories,
									this->pre_obs_histories,
									this->post_obs_histories,
									this->target_val_histories,
									this->signals,
									this->miss_average_guess);

		if (this->signals.size() > 0) {
			this->state = SIGNAL_EXPERIMENT_STATE_DONE;
		} else {
			this->pre_actions.clear();
			this->post_actions.clear();

			this->new_scores.clear();

			this->pre_obs_histories.clear();
			this->post_obs_histories.clear();
			this->target_val_histories.clear();

			geometric_distribution<int> num_actions_distribution(0.2);
			uniform_int_distribution<int> action_distribution(0, 2);
			int num_pre = num_actions_distribution(generator);
			for (int a_index = 0; a_index < num_pre; a_index++) {
				this->pre_actions.push_back(action_distribution(generator));
			}
			int num_post = 5 + num_actions_distribution(generator);
			for (int a_index = 0; a_index < num_post; a_index++) {
				this->post_actions.push_back(action_distribution(generator));
			}

			this->state = SIGNAL_EXPERIMENT_STATE_FIND_SAFE;
		}
	}
}
