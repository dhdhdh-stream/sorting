#include "signal_experiment.h"

#include <iostream>

#include "globals.h"
#include "signal.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_MIN_EXISTING_NUM_SAMPLES = 10;
const int INITIAL_C1_NUM_SAMPLES = 1;
const int INITIAL_C2_NUM_SAMPLES = 2;
const int INITIAL_C3_NUM_SAMPLES = 3;
const int INITIAL_C4_NUM_SAMPLES = 4;
#else
const int INITIAL_MIN_EXISTING_NUM_SAMPLES = 100;
const int INITIAL_C1_NUM_SAMPLES = 2;
const int INITIAL_C2_NUM_SAMPLES = 10;
const int INITIAL_C3_NUM_SAMPLES = 40;
const int INITIAL_C4_NUM_SAMPLES = 100;
#endif /* MDEBUG */

void SignalExperiment::initial_backprop(double target_val,
										SignalExperimentHistory* history,
										SolutionWrapper* wrapper) {
	bool is_eval = false;
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C1:
		if (this->new_scores.size() >= INITIAL_C1_NUM_SAMPLES
				&& this->existing_scores.size() >= INITIAL_MIN_EXISTING_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C2:
		if (this->new_scores.size() >= INITIAL_C2_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C3:
		if (this->new_scores.size() >= INITIAL_C3_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C4:
		if (this->new_scores.size() >= INITIAL_C4_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	}

	if (is_eval) {
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

		#if defined(MDEBUG) && MDEBUG
		if (new_score_average >= existing_score_average || rand()%5 != 0) {
		#else
		if (new_score_average >= existing_score_average) {
		#endif /* MDEBUG */
			switch (this->state) {
			case SIGNAL_EXPERIMENT_STATE_INITIAL_C1:
				this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C2;
				break;
			case SIGNAL_EXPERIMENT_STATE_INITIAL_C2:
				this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C3;
				break;
			case SIGNAL_EXPERIMENT_STATE_INITIAL_C3:
				this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C4;
				break;
			case SIGNAL_EXPERIMENT_STATE_INITIAL_C4:
				this->existing_scores.clear();
				this->new_scores.clear();

				this->curr_ramp = 0;

				this->state = SIGNAL_EXPERIMENT_STATE_RAMP;
				this->state_iter = 0;
				this->num_fail = 0;

				break;
			}
		} else {
			for (int s_index = 0; s_index < (int)this->adjusted_previous_pre_signals.size(); s_index++) {
				delete this->adjusted_previous_pre_signals[s_index];
			}
			this->adjusted_previous_pre_signals.clear();
			for (int s_index = 0; s_index < (int)this->adjusted_previous_post_signals.size(); s_index++) {
				delete this->adjusted_previous_post_signals[s_index];
			}
			this->adjusted_previous_post_signals.clear();

			this->existing_scores.clear();
			this->new_scores.clear();

			this->existing_explore_pre_obs.clear();
			this->existing_explore_post_obs.clear();
			this->existing_explore_scores.clear();

			this->new_explore_pre_obs.clear();
			this->new_explore_post_obs.clear();
			this->new_explore_scores.clear();

			set_actions();

			this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C1;
			this->state_iter = 0;
		}
	}
}
