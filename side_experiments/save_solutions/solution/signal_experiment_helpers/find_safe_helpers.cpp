#include "signal_experiment.h"

#include <cmath>
#include <iostream>

#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "signal_instance.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int CHECK_1_NUM_ITERS = 1;
const int CHECK_2_NUM_ITERS = 2;
const int CHECK_3_NUM_ITERS = 4;
#else
const int CHECK_1_NUM_ITERS = 1;
const int CHECK_2_NUM_ITERS = 10;
const int CHECK_3_NUM_ITERS = 100;
#endif /* MDEBUG */

const double CURRENT_MIN_T_SCORE = -0.674;
const double AVERAGE_MIN_T_SCORE = -0.674;
const double INDIVIDUAL_MIN_T_SCORE = -2.326;

bool SignalExperiment::find_safe_check_signal(vector<double>& obs,
											  int& action,
											  bool& is_next,
											  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	/**
	 * - check pre
	 */
	if (scope_history->node_histories.size() == 0) {
		if (scope_history->signal_pre_obs.size() == 0) {
			SignalExperimentInstanceHistory* new_instance_history = new SignalExperimentInstanceHistory();
			new_instance_history->scope_history = scope_history;
			/**
			 * - start from layer above
			 */
			for (int l_index = (int)wrapper->scope_histories.size()-2; l_index >= 0; l_index--) {
				map<int, Signal*>::iterator it = wrapper->signals.find(wrapper->scope_histories[l_index]->scope->id);
				if (it != wrapper->signals.end()) {
					new_instance_history->signal_needed_from = wrapper->scope_histories[l_index];
					break;
				}
			}
			wrapper->signal_experiment_instance_histories.push_back(new_instance_history);
		}

		scope_history->signal_pre_obs.push_back(obs);

		if (scope_history->signal_pre_obs.size() <= this->pre_actions.size()) {
			action = this->pre_actions[scope_history->signal_pre_obs.size()-1];
			is_next = true;

			wrapper->num_actions++;

			return true;
		}
	}

	/**
	 * - check post
	 */
	if (wrapper->node_context.back() == NULL
			&& wrapper->experiment_context.back() == NULL) {
		scope_history->signal_post_obs.push_back(obs);

		if (scope_history->signal_post_obs.size() <= this->post_actions.size()) {
			action = this->post_actions[scope_history->signal_post_obs.size()-1];
			is_next = true;

			wrapper->num_actions++;

			return true;
		}
	}

	return false;
}

void SignalExperiment::find_safe_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	if (this->solution_type == SIGNAL_EXPERIMENT_SOLUTION_TYPE_CURRENT) {
		this->new_current_scores.push_back(target_val);

		for (int i_index = 0; i_index < (int)wrapper->signal_experiment_instance_histories.size(); i_index++) {
			SignalExperimentInstanceHistory* instance_history = wrapper->signal_experiment_instance_histories[i_index];

			this->current_pre_obs.push_back(instance_history->scope_history->signal_pre_obs);
			this->current_post_obs.push_back(instance_history->scope_history->signal_post_obs);

			double inner_target_val;
			if (instance_history->signal_needed_from == NULL) {
				inner_target_val = target_val;
			} else {
				inner_target_val = calc_signal(instance_history->signal_needed_from,
											   wrapper);
			}
			this->current_scores.push_back(inner_target_val);
		}

		this->solution_type = SIGNAL_EXPERIMENT_SOLUTION_TYPE_POSITIVE;
		this->solution_index = 0;
	} else {
		this->new_positive_scores[this->solution_index].push_back(target_val);

		for (int i_index = 0; i_index < (int)wrapper->signal_experiment_instance_histories.size(); i_index++) {
			SignalExperimentInstanceHistory* instance_history = wrapper->signal_experiment_instance_histories[i_index];

			this->positive_pre_obs.push_back(instance_history->scope_history->signal_pre_obs);
			this->positive_post_obs.push_back(instance_history->scope_history->signal_post_obs);

			double inner_target_val;
			if (instance_history->signal_needed_from == NULL) {
				inner_target_val = target_val;
			} else {
				inner_target_val = calc_signal(instance_history->signal_needed_from,
											   wrapper);
			}
			this->positive_scores.push_back(inner_target_val);
		}

		this->solution_index++;
		if (this->solution_index >= (int)wrapper->positive_solutions.size()) {
			this->solution_index = 0;
			this->state_iter++;

			bool is_fail = false;
			if (this->state_iter == CHECK_1_NUM_ITERS
					|| this->state_iter == CHECK_2_NUM_ITERS
					|| this->state_iter == CHECK_3_NUM_ITERS) {
				double current_new_sum_val = 0.0;
				for (int h_index = 0; h_index < (int)this->new_current_scores.size(); h_index++) {
					current_new_sum_val += this->new_current_scores[h_index];
				}
				double current_new_val_average = current_new_sum_val / (double)this->new_current_scores.size();

				double current_val_diff = current_new_val_average - wrapper->solution->curr_val_average;
				double current_t_score = current_val_diff / (wrapper->solution->curr_val_standard_deviation
					/ sqrt((double)this->new_current_scores.size()));

				if (current_t_score < CURRENT_MIN_T_SCORE) {
					is_fail = true;
				}

				double sum_normalized = 0.0;
				for (int s_index = 0; s_index < (int)wrapper->positive_solutions.size(); s_index++) {
					double sum_vals = 0.0;
					for (int h_index = 0; h_index < (int)this->new_positive_scores[s_index].size(); h_index++) {
						sum_vals += this->new_positive_scores[s_index][h_index];
					}
					double new_val_average = sum_vals / (double)this->new_positive_scores[s_index].size();

					double val_diff = new_val_average - this->existing_positive_score_averages[s_index];

					sum_normalized += val_diff / this->existing_positive_score_standard_deviations[s_index];
				}

				int num_samples = this->state_iter * (int)wrapper->positive_solutions.size();
				double normalized_average = sum_normalized / (double)num_samples;
				double t_score = normalized_average * sqrt(num_samples);
				if (t_score < AVERAGE_MIN_T_SCORE) {
					is_fail = true;
				}
			}
			if (this->state_iter == CHECK_3_NUM_ITERS) {
				for (int s_index = 0; s_index < (int)wrapper->positive_solutions.size(); s_index++) {
					double sum_vals = 0.0;
					for (int h_index = 0; h_index < (int)this->new_positive_scores[s_index].size(); h_index++) {
						sum_vals += this->new_positive_scores[s_index][h_index];
					}
					double new_val_average = sum_vals / (double)this->new_positive_scores[s_index].size();

					double val_diff = new_val_average - this->existing_positive_score_averages[s_index];
					double t_score = val_diff / (this->existing_positive_score_standard_deviations[s_index]
						/ sqrt((double)this->new_positive_scores[s_index].size()));

					if (t_score < INDIVIDUAL_MIN_T_SCORE) {
						is_fail = true;
						break;
					}
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if (is_fail && rand()%2 == 0) {
			#else
			if (is_fail) {
			#endif /* MDEBUG */
				this->pre_actions.clear();
				this->post_actions.clear();

				this->new_current_scores.clear();
				this->new_positive_scores.clear();

				this->current_pre_obs.clear();
				this->current_post_obs.clear();
				this->current_scores.clear();

				this->positive_pre_obs.clear();
				this->positive_post_obs.clear();
				this->positive_scores.clear();

				for (int s_index = 0; s_index < (int)this->instances.size(); s_index++) {
					delete this->instances[s_index];
				}
				this->instances.clear();

				set_actions(wrapper);

				this->state_iter = 0;
				this->solution_type = SIGNAL_EXPERIMENT_SOLUTION_TYPE_CURRENT;
			} else if (this->state_iter >= CHECK_3_NUM_ITERS) {
				// temp
				cout << "pre_actions:";
				for (int a_index = 0; a_index < (int)this->pre_actions.size(); a_index++) {
					cout << " " << this->pre_actions[a_index];
				}
				cout << endl;
				cout << "post_actions:";
				for (int a_index = 0; a_index < (int)this->post_actions.size(); a_index++) {
					cout << " " << this->post_actions[a_index];
				}
				cout << endl;

				if (wrapper->trap_solutions.size() > 0) {
					// temp
					cout << "SIGNAL_EXPERIMENT_STATE_GATHER_TRAP" << endl;
					this->state = SIGNAL_EXPERIMENT_STATE_GATHER_TRAP;
					this->state_iter = 0;
					this->solution_type = SIGNAL_EXPERIMENT_SOLUTION_TYPE_TRAP;
					this->solution_index = 0;
				} else {
					// temp
					cout << "SIGNAL_EXPERIMENT_STATE_GATHER_CURRENT" << endl;
					this->state = SIGNAL_EXPERIMENT_STATE_GATHER_CURRENT;
					this->state_iter = 0;
					this->solution_type = SIGNAL_EXPERIMENT_SOLUTION_TYPE_CURRENT;
				}
			}
		}
	}
}