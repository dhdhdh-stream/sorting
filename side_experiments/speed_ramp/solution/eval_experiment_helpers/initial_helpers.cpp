#include "eval_experiment.h"

#include <cmath>
#include <iostream>

#include "abstract_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES = 2;
#else
const int INITIAL_NUM_SAMPLES = 100;
#endif /* MDEBUG */

void EvalExperiment::initial_backprop(double target_val,
									  EvalExperimentHistory* history,
									  SolutionWrapper* wrapper) {
	if (history->is_on) {
		this->new_scores.push_back(target_val);
	} else {
		this->existing_scores.push_back(target_val);
	}

	if (this->new_scores.size() >= INITIAL_NUM_SAMPLES) {
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
		if (new_score_average >= existing_score_average || rand()%3 != 0) {
		#else
		if (new_score_average >= existing_score_average) {
		#endif /* MDEBUG */
			this->existing_scores.clear();
			this->new_scores.clear();

			this->curr_ramp = 0;

			this->state = EVAL_EXPERIMENT_STATE_RAMP;
			this->state_iter = 0;
			this->num_fail = 0;
		} else {
			this->num_fail++;
			if (this->num_fail >= 2) {
				this->node_context->experiment = NULL;
				delete this;
			} else {
				this->existing_scores.clear();
				this->new_scores.clear();
			}
		}
	}
}
