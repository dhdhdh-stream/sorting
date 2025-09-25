#include "eval_experiment.h"

#include <cmath>
#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "helpers.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void EvalExperiment::ramp_backprop(double target_val,
								   EvalExperimentHistory* history,
								   SolutionWrapper* wrapper) {
	if (history->is_on) {
		this->new_scores.push_back(target_val);

		for (int i_index = 0; i_index < (int)history->signal_is_set.size(); i_index++) {
			if (history->signal_is_set[i_index]) {
				this->new_signals.push_back(history->signal_vals[i_index]);
			} else {
				this->new_signals.push_back(target_val);
			}
		}
	} else {
		this->existing_scores.push_back(target_val);

		for (int i_index = 0; i_index < (int)history->signal_is_set.size(); i_index++) {
			if (history->signal_is_set[i_index]) {
				this->existing_signals.push_back(history->signal_vals[i_index]);
			} else {
				this->existing_signals.push_back(target_val);
			}
		}
	}

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

				this->state = EVAL_EXPERIMENT_STATE_GATHER;
				this->state_iter = 0;
			}
		} else {
			this->num_fail++;
			if (this->num_fail >= 2) {
				this->curr_ramp--;
				this->num_fail = 0;

				if (this->curr_ramp < 0) {
					this->node_context->experiment = NULL;
					delete this;
				}
			}
		}
	}
}
