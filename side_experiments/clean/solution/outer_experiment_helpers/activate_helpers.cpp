#include "outer_experiment.h"

#include "globals.h"

using namespace std;

bool OuterExperiment::activate(Problem* problem,
							   RunHelper& run_helper) {
	run_helper.experiments_seen_order.push_back(this);
	run_helper.experiments_seen.insert(this);

	double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
	uniform_real_distribution<double> distribution(0.0, 1.0);
	if (distribution(generator) < selected_probability) {
		run_helper.experiment_history = new OuterExperimentOverallHistory(this);

		switch (this->state) {
		case OUTER_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
			measure_existing_score_activate(problem,
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
		case OUTER_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
			measure_new_score_activate(problem,
									   run_helper);
			break;
		case OUTER_EXPERIMENT_STATE_VERIFY_1ST_EXISTING_SCORE:
		case OUTER_EXPERIMENT_STATE_VERIFY_2ND_EXISTING_SCORE:
			verify_existing_score_activate(problem,
										   run_helper);
			break;
		case OUTER_EXPERIMENT_STATE_VERIFY_1ST_NEW_SCORE:
		case OUTER_EXPERIMENT_STATE_VERIFY_2ND_NEW_SCORE:
			verify_new_score_activate(problem,
									  run_helper);
			break;
		#if defined(MDEBUG) && MDEBUG
		case OUTER_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(problem,
									run_helper);
			break;
		#endif /* MDEBUG */
		}

		return true;
	} else {
		return false;
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
	case OUTER_EXPERIMENT_STATE_VERIFY_1ST_EXISTING_SCORE:
	case OUTER_EXPERIMENT_STATE_VERIFY_2ND_EXISTING_SCORE:
		verify_existing_score_backprop(target_val,
									   run_helper);
		break;
	case OUTER_EXPERIMENT_STATE_VERIFY_1ST_NEW_SCORE:
	case OUTER_EXPERIMENT_STATE_VERIFY_2ND_NEW_SCORE:
		verify_new_score_backprop(target_val);
		break;
	#if defined(MDEBUG) && MDEBUG
	case OUTER_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
