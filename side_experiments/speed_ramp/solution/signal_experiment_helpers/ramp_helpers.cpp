#include "signal_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "signal.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EVAL_GEAR = 2;
#else
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

		double new_sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			new_sum_score += this->new_scores[h_index];
		}
		double new_score_average = new_sum_score / (double)this->new_scores.size();

		this->existing_scores.clear();
		this->new_scores.clear();

		this->state_iter = 0;

		#if defined(MDEBUG) && MDEBUG
		if (new_score_average >= existing_score_average || rand()%3 != 0) {
		#else
		if (new_score_average >= existing_score_average) {
		#endif /* MDEBUG */
			this->curr_ramp++;
			this->num_fail = 0;

			if (this->curr_ramp == EVAL_GEAR) {
				this->curr_ramp--;

				this->state = SIGNAL_EXPERIMENT_STATE_GATHER;
			}
		} else {
			this->num_fail++;
			if (this->num_fail >= 2) {
				this->curr_ramp--;
				this->num_fail = 0;

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
}
