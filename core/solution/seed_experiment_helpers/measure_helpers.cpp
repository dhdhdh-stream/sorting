#include "seed_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "solution.h"

using namespace std;

void SeedExperiment::measure_backprop(double target_val) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state_iter >= solution->curr_num_datapoints) {
		this->combined_score /= solution->curr_num_datapoints;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double combined_improvement = this->combined_score - this->existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (this->existing_score_standard_deviation / sqrt(solution->curr_num_datapoints));

		if (combined_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			this->combined_score = 0.0;

			this->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

			this->state = SEED_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;
			this->state_iter = 0;
		} else {
			this->state = SEED_EXPERIMENT_STATE_FIND_GATHER;
			this->state_iter = 0;
			this->sub_state_iter = -1;
		}
	}
}
