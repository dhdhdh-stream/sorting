#include "signal_experiment.h"

#include <cmath>

#include "helpers.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const int CHECK_1_NUM_ITERS = 2;
const int CHECK_2_NUM_ITERS = 20;
const int CHECK_3_NUM_ITERS = 200;

const double AVERAGE_MIN_T_SCORE = -0.674;
const double INDIVIDUAL_MIN_T_SCORE = -1.282;

void SignalExperiment::find_safe_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	this->new_scores[this->solution_index].push_back(target_val);

	for (int i_index = 0; i_index < (int)wrapper->signal_experiment_instance_histories.size(); i_index++) {
		SignalExperimentInstanceHistory* instance_history = wrapper->signal_experiment_instance_histories[i_index];

		this->existing_pre_obs[this->solution_index].push_back(instance_history->scope_history->signal_pre_obs);
		this->existing_post_obs[this->solution_index].push_back(instance_history->scope_history->signal_post_obs);

		double inner_target_val;
		if (instance_history->signal_needed_from == NULL) {
			inner_target_val = target_val;
		} else {
			inner_target_val = calc_signal(instance_history->signal_needed_from,
										   wrapper);
		}
		this->existing_scores[this->solution_index].push_back(inner_target_val);
	}

	this->solution_index++;
	if (this->solution_index >= (int)wrapper->solutions.size()) {
		this->solution_index = 0;
		this->state_iter++;

		bool is_fail = false;
		if (this->state_iter == CHECK_1_NUM_ITERS
				|| this->state_iter == CHECK_2_NUM_ITERS
				|| this->state_iter == CHECK_3_NUM_ITERS) {
			double sum_t_scores = 0.0;
			for (int s_index = 0; s_index < (int)wrapper->solutions.size(); s_index++) {
				double sum_vals = 0.0;
				for (int h_index = 0; h_index < (int)this->new_scores[s_index].size(); h_index++) {
					sum_vals += this->new_scores[s_index][h_index];
				}
				double new_val_average = sum_vals / (double)this->new_scores[s_index].size();

				double val_diff = new_val_average - wrapper->solutions[s_index]->curr_val_average;
				double t_score = val_diff / (wrapper->solutions[s_index]->curr_val_standard_deviation
					/ sqrt((double)this->new_scores[s_index].size()));

				if (t_score < INDIVIDUAL_MIN_T_SCORE) {
					is_fail = true;
					break;
				}

				sum_t_scores += t_score;
			}

			double t_score_val_average = sum_t_scores / (double)wrapper->solutions.size();
			if (t_score_val_average < AVERAGE_MIN_T_SCORE) {
				is_fail = true;
			}
		}

		if (is_fail) {

		} else if (this->state_iter >= CHECK_3_NUM_ITERS) {

		}
	}
}