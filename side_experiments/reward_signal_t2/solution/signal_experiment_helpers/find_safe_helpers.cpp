#include "signal_experiment.h"

#include <cmath>

#include "constants.h"
#include "globals.h"
#include "problem.h"

using namespace std;

void SignalExperiment::find_safe_pre_activate(
		Problem* problem) {
	for (int a_index = 0; a_index < (int)this->pre_actions.size(); a_index++) {
		problem->perform_action(this->pre_actions[a_index]);
	}
}

void SignalExperiment::find_safe_post_activate(
		Problem* problem) {
	for (int a_index = 0; a_index < (int)this->post_actions.size(); a_index++) {
		problem->perform_action(this->post_actions[a_index]);
	}
}

void SignalExperiment::find_safe_backprop(
		double target_val) {
	this->new_scores.push_back(target_val);

	if (this->new_scores.size() >= MEASURE_ITERS) {
		double sum_existing_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
			sum_existing_vals += this->existing_scores[h_index];
		}
		double existing_average = sum_existing_vals / (double)this->existing_scores.size();

		double sum_existing_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
			sum_existing_variance += (this->existing_scores[h_index] - existing_average)
				* (this->existing_scores[h_index] - existing_average);
		}
		double existing_standard_deviation = sqrt(sum_existing_variance / (double)this->existing_scores.size());

		double sum_new_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			sum_new_vals += this->new_scores[h_index];
		}
		double new_average = sum_new_vals / (double)this->new_scores.size();

		double sum_new_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			sum_existing_variance += (this->new_scores[h_index] - new_average)
				* (this->new_scores[h_index] - new_average);
		}
		double new_standard_deviation = sqrt(sum_new_variance / (double)this->new_scores.size());

		double new_improvement = new_average - existing_average;
		double min_standard_deviation = min(existing_standard_deviation, new_standard_deviation);
		double t_score = new_improvement / (min_standard_deviation / sqrt(MEASURE_ITERS));

		if (t_score > -0.674) {
			this->state = SIGNAL_EXPERIMENT_STATE_EXPLORE;
		} else {
			this->pre_actions.clear();
			this->post_actions.clear();

			this->new_scores.clear();

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
		}
	}
}
