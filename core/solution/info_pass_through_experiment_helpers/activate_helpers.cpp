#include "info_pass_through_experiment.h"

#include "globals.h"

using namespace std;

void InfoPassThroughExperiment::info_pre_activate(
		RunHelper& run_helper) {
	if (run_helper.experiment_histories.size() == 0) {
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
				run_helper.experiment_histories.push_back(new InfoPassThroughExperimentHistory(this));
			}

			run_helper.experiments_seen_order.push_back(this);
		}
	}
}

bool InfoPassThroughExperiment::activate(AbstractNode* experiment_node,
										 bool is_branch,
										 AbstractNode*& curr_node,
										 Problem* problem,
										 vector<ContextLayer>& context,
										 RunHelper& run_helper) {
	if (run_helper.experiment_histories.back()->experiment == this) {
		InfoPassThroughExperimentHistory* history = (InfoPassThroughExperimentHistory*)run_helper.experiment_histories.back();

		switch (this->state) {
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
			measure_existing_activate(history);
			break;
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem);
			break;
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_activate(curr_node);
			break;
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_activate(curr_node);
			break;
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node);
			break;
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY:
			verify_activate(curr_node);
			break;
		#if defined(MDEBUG) && MDEBUG
		case INFO_PASS_THROUGH_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(curr_node);
			break;
		#endif /* MDEBUG */
		}

		return true;
	} else {
		return false;
	}
}

bool InfoPassThroughExperiment::info_back_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		bool& is_positive) {
	switch (this->state) {
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_info_back_activate(context,
											run_helper);
		return false;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
		explore_info_back_activate(context,
								   run_helper);
		return false;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_EXISTING:
		return train_existing_info_back_activate(context,
												 run_helper,
												 is_positive);
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW:
		return train_new_info_back_activate(context,
											run_helper,
											is_positive);
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
		measure_info_back_activate(context,
								   run_helper,
								   is_positive);
		return true;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_info_back_activate(context,
										   run_helper);
		return false;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY:
		verify_info_back_activate(context,
								  run_helper,
								  is_positive);
		return true;
	#if defined(MDEBUG) && MDEBUG
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_info_back_activate(problem,
										  context,
										  run_helper,
										  is_positive);
		return true;
	#endif /* MDEBUG */
	}

	return false;
}

void InfoPassThroughExperiment::back_activate(vector<ContextLayer>& context,
											  RunHelper& run_helper) {
	switch (this->state) {
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_back_activate(context,
									   run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
		explore_back_activate(context,
							  run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_back_activate(context,
									 run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_back_activate(context,
								run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
		measure_back_activate(context,
							  run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_back_activate(context,
									  run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY:
		verify_back_activate(context,
							 run_helper);
		break;
	}
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
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY:
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
