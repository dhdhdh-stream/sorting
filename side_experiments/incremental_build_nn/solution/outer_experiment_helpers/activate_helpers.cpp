#include "outer_experiment.h"

#include "globals.h"

using namespace std;

bool OuterExperiment::activate(Problem* problem,
							   RunHelper& run_helper) {
	run_helper.experiments_seen_order.push_back(this);

	double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
	uniform_real_distribution<double> distribution(0.0, 1.0);
	if (distribution(generator) < selected_probability) {
		run_helper.experiment_history = new OuterExperimentOverallHistory(this);

		switch (this->state) {
		case OUTER_EXPERIMENT_STATE_MEASURE_EXISTING:
			measure_existing_activate(problem,
									  run_helper);
			break;
		case OUTER_EXPERIMENT_STATE_EXPLORE:
			if (this->sub_state_iter == 0) {
				explore_initial_activate(problem,
										 run_helper);
			} else {
				explore_activate(problem,
								 run_helper);
			}
			break;
		case OUTER_EXPERIMENT_STATE_MEASURE_NEW:
			measure_new_activate(problem,
								 run_helper);
			break;
		case OUTER_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
		case OUTER_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
			verify_existing_activate(problem,
									 run_helper);
			break;
		case OUTER_EXPERIMENT_STATE_VERIFY_1ST_NEW:
		case OUTER_EXPERIMENT_STATE_VERIFY_2ND_NEW:
			verify_new_activate(problem,
								run_helper);
			break;
		}

		return true;
	} else {
		return false;
	}
}

void OuterExperiment::backprop(double target_val,
							   RunHelper& run_helper,
							   AbstractExperimentHistory* history) {
	switch (this->state) {
	case OUTER_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper);
		break;
	case OUTER_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val);
		break;
	case OUTER_EXPERIMENT_STATE_MEASURE_NEW:
		measure_new_backprop(target_val);
		break;
	case OUTER_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case OUTER_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case OUTER_EXPERIMENT_STATE_VERIFY_1ST_NEW:
	case OUTER_EXPERIMENT_STATE_VERIFY_2ND_NEW:
		verify_new_backprop(target_val);
		break;
	}
}
