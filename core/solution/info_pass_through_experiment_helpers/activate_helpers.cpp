#include "info_pass_through_experiment.h"

#include "globals.h"

using namespace std;

bool InfoPassThroughExperiment::activate(AbstractNode*& curr_node,
										 Problem* problem,
										 vector<ContextLayer>& context,
										 RunHelper& run_helper) {
	bool is_selected = false;
	InfoPassThroughExperimentHistory* history = NULL;
	if (run_helper.experiment_histories.size() == 1
			&& run_helper.experiment_histories[0]->experiment == this) {
		history = (InfoPassThroughExperimentHistory*)run_helper.experiment_histories[0];
		is_selected = true;
	} else if (run_helper.experiment_histories.size() == 0) {
		bool has_seen = false;
		for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
			if (run_helper.experiments_seen_order[e_index] == this) {
				has_seen = true;
				break;
			}
		}
		if (!has_seen) {
			double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
			uniform_real_distribution<double> distribution(0.0, 1.0);
			if (distribution(generator) < selected_probability) {
				history = new InfoPassThroughExperimentHistory(this);
				run_helper.experiment_histories.push_back(history);
				is_selected = true;
			}

			run_helper.experiments_seen_order.push_back(this);
		}
	}

	if (is_selected) {
		switch (this->state) {
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
			measure_existing_activate(history);
			break;
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 run_helper);
			break;
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEGATIVE:
			train_negative_activate(curr_node,
									problem,
									context,
									run_helper,
									history);
			break;
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_POSITIVE:
			train_positive_activate(curr_node,
									problem,
									context,
									run_helper,
									history);
			break;
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node,
							 problem,
							 context,
							 run_helper,
							 history);
			break;
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
			verify_activate(curr_node,
							problem,
							context,
							run_helper,
							history);
			break;
		#if defined(MDEBUG) && MDEBUG
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(curr_node,
									problem,
									context,
									run_helper,
									history);
			break;
		#endif /* MDEBUG */
		}

		return true;
	} else {
		return false;
	}
}

bool InfoPassThroughExperiment::back_activate(
		Problem* problem,
		ScopeHistory*& subscope_history,
		bool& result_is_positive,
		RunHelper& run_helper) {
	switch (this->state) {
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEGATIVE:
		train_negative_back_activate(subscope_history,
									 result_is_positive,
									 run_helper);
		return true;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_POSITIVE:
		train_positive_back_activate(subscope_history,
									 result_is_positive,
									 run_helper);
		return true;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
		measure_back_activate(subscope_history,
							  result_is_positive,
							  run_helper);
		return true;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
		verify_back_activate(subscope_history,
							 result_is_positive,
							 run_helper);
		return true;
	#if defined(MDEBUG) && MDEBUG
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_back_activate(problem,
									 subscope_history,
									 result_is_positive,
									 run_helper);
		return true;
	#endif /* MDEBUG */
	}

	return false;
}

void InfoPassThroughExperiment::backprop(double target_val,
										 RunHelper& run_helper) {
	switch (this->state) {
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEGATIVE:
		train_negative_backprop(target_val,
								run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_POSITIVE:
		train_positive_backprop(target_val,
								run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
		verify_backprop(target_val,
						run_helper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
