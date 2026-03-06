#include "repetition_experiment.h"

#include <iostream>

#include "constants.h"
#include "solution_wrapper.h"

using namespace std;

void RepetitionExperiment::measure_existing_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	RepetitionExperimentHistory* history = (RepetitionExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		this->existing_scores.push_back(target_val);

		this->state_iter++;
		if (this->state_iter >= MEASURE_STEP_NUM_ITERS) {
			this->state = REPETITION_EXPERIMENT_STATE_MEASURE_NEW;
			this->state_iter = 0;
		}
	}
}
