#include "outer_experiment.h"

using namespace std;

void OuterExperiment::activate(Problem& problem,
							   RunHelper& run_helper) {
	run_helper.experiment_history = this;
	/**
	 * - to prevent other experiments
	 */

	switch (this->state) {
	case OUTER_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
		measure_existing_score_activate(problem,
										run_helper);
		break;
	case OUTER_EXPERIMENT_STATE_EXPLORE:
		explore_activate(problem,
						 run_helper);
		break;
	case OUTER_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
		measure_new_score_activate(problem,
								   run_helper);
		break;
	}
}

void OuterExperiment::backprop(double target_val,
							   RunHelper& run_helper) {
	switch (this->state) {
	case OUTER_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
		measure_existing_score_backprop(target_val,
										run_helper);
		break;
	case OUTER_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val);
		break;
	case OUTER_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
		measure_new_score_backprop(target_val);
		break;
	}
}
