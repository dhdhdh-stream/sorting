#include "retrain_branch_experiment.h"

#include <iostream>

#include "globals.h"

using namespace std;

bool RetrainBranchExperiment::activate(bool& is_branch,
									   Problem* problem,
									   vector<ContextLayer>& context,
									   RunHelper& run_helper) {
	bool is_selected = false;
	if (run_helper.selected_experiment == NULL) {
		bool select = false;
		set<AbstractExperiment*>::iterator it = run_helper.experiments_seen.find(this);
		if (it == run_helper.experiments_seen.end()) {
			double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
			uniform_real_distribution<double> distribution(0.0, 1.0);
			if (distribution(generator) < selected_probability) {
				select = true;
			}

			run_helper.experiments_seen_order.push_back(this);
			run_helper.experiments_seen.insert(this);
		}
		if (select) {
			run_helper.selected_experiment = this;
			run_helper.experiment_history = new RetrainBranchExperimentOverallHistory(this);

			is_selected = true;
		}
	} else if (run_helper.selected_experiment == this) {
		is_selected = true;
	}

	if (is_selected) {
		switch (this->state) {
		case RETRAIN_BRANCH_EXPERIMENT_STATE_TRAIN_ORIGINAL:
			train_original_activate(is_branch,
									context,
									run_helper);
			break;
		case RETRAIN_BRANCH_EXPERIMENT_STATE_TRAIN_BRANCH:
			train_branch_activate(is_branch,
								  context,
								  run_helper);
			break;
		case RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE_EXISTING:
			measure_existing_activate(is_branch,
									  context,
									  run_helper);
			break;
		case RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE:
			measure_activate(is_branch,
							 context,
							 run_helper);
			break;
		case RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_EXISTING:
			verify_existing_activate(is_branch,
									 context,
									 run_helper);
			break;
		case RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY:
			verify_activate(is_branch,
							context,
							run_helper);
			break;
		#if defined(MDEBUG) && MDEBUG
		case RETRAIN_BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(is_branch,
									problem,
									context,
									run_helper);
			break;
		#endif /* MDEBUG */
		}

		return true;
	} else {
		return false;
	}
}

void RetrainBranchExperiment::backprop(double target_val,
									   RunHelper& run_helper,
									   RetrainBranchExperimentOverallHistory* history) {
	switch (this->state) {
	case RETRAIN_BRANCH_EXPERIMENT_STATE_TRAIN_ORIGINAL:
		train_original_backprop(target_val,
								history);
		break;
	case RETRAIN_BRANCH_EXPERIMENT_STATE_TRAIN_BRANCH:
		train_branch_backprop(target_val,
							  history);
		break;
	case RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper);
		break;
	case RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val);
		break;
	case RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY:
		verify_backprop(target_val);
		break;
	#if defined(MDEBUG) && MDEBUG
	case RETRAIN_BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}

	delete history;
}