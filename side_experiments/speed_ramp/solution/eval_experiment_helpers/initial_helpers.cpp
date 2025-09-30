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
		this->new_sum_scores += target_val;
		this->new_count++;
	} else {
		this->existing_sum_scores += target_val;
		this->existing_count++;
	}

	if (this->new_count >= INITIAL_NUM_SAMPLES) {
		double existing_score_average = this->existing_sum_scores / (double)this->existing_count;
		double new_score_average = this->new_sum_scores / (double)this->new_count;
		#if defined(MDEBUG) && MDEBUG
		if (new_score_average >= existing_score_average || rand()%3 != 0) {
		#else
		if (new_score_average >= existing_score_average) {
		#endif /* MDEBUG */
			this->existing_sum_scores = 0.0;
			this->existing_count = 0;
			this->new_sum_scores = 0.0;
			this->new_count = 0;

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
				this->existing_sum_scores = 0.0;
				this->existing_count = 0;
				this->new_sum_scores = 0.0;
				this->new_count = 0;
			}
		}
	}
}
