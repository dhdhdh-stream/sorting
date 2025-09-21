#include "signal_experiment.h"

#include <cmath>
#include <iostream>

#include "signal.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int RAMP_EPOCH_NUM_ITERS = 20;

const int EVAL_GEAR = 2;
#else
const int RAMP_EPOCH_NUM_ITERS = 8000;

const int EVAL_GEAR = 5;
#endif /* MDEBUG */

void SignalExperiment::ramp_backprop(double target_val,
									 SignalExperimentHistory* history,
									 SolutionWrapper* wrapper) {
	this->state_iter++;
	#if defined(MDEBUG) && MDEBUG
	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS
			&& this->existing_scores.size() >= 2
			&& this->new_scores.size() >= 2) {
	#else
	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
	#endif /* MDEBUG */
		double existing_sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
			existing_sum_score += this->existing_scores[h_index];
		}
		double existing_score_average = existing_sum_score / (double)this->existing_scores.size();

		double existing_sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
			existing_sum_variance += (this->existing_scores[h_index] - existing_score_average)
				* (this->existing_scores[h_index] - existing_score_average);
		}
		double existing_score_standard_deviation = sqrt(existing_sum_variance / (double)this->existing_scores.size());

		double new_sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			new_sum_score += this->new_scores[h_index];
		}
		double new_score_average = new_sum_score / (double)this->new_scores.size();

		double new_sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			new_sum_variance += (this->new_scores[h_index] - new_score_average)
				* (this->new_scores[h_index] - new_score_average);
		}
		double new_score_standard_deviation = sqrt(new_sum_variance / (double)this->new_scores.size());

		double score_improvement = new_score_average - existing_score_average;
		double existing_score_standard_error = existing_score_standard_deviation / sqrt((double)this->existing_scores.size());
		double new_score_standard_error = new_score_standard_deviation / sqrt((double)this->new_scores.size());
		double score_t_score = score_improvement / sqrt(
			existing_score_standard_error * existing_score_standard_error
				+ new_score_standard_error * new_score_standard_error);

		this->existing_scores.clear();
		this->new_scores.clear();

		this->state_iter = 0;

		#if defined(MDEBUG) && MDEBUG
		if (score_t_score >= -0.674 || rand()%3 != 0) {
		#else
		if (score_t_score >= -0.674) {
		#endif /* MDEBUG */
			this->curr_ramp++;
			if (this->curr_ramp == EVAL_GEAR) {
				this->curr_ramp--;

				this->state = SIGNAL_EXPERIMENT_STATE_GATHER;
			}
		} else {
			this->curr_ramp--;
			if (this->curr_ramp < 0) {
				for (int s_index = 0; s_index < (int)this->adjusted_previous_signals.size(); s_index++) {
					delete this->adjusted_previous_signals[s_index];
				}
				this->adjusted_previous_signals.clear();

				this->existing_scores.clear();
				this->new_scores.clear();

				this->new_current_pre_obs.clear();
				this->new_current_post_obs.clear();
				this->new_current_scores.clear();

				this->new_explore_pre_obs.clear();
				this->new_explore_post_obs.clear();
				this->new_explore_scores.clear();

				set_actions();

				this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C1;
				this->state_iter = 0;
			}
		}
	}
}
