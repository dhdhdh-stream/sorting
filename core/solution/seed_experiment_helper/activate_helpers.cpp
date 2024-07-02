#include "seed_experiment.h"

#include "globals.h"

using namespace std;

bool SeedExperiment::activate(AbstractNode* experiment_node,
							  bool is_branch,
							  AbstractNode*& curr_node,
							  Problem* problem,
							  vector<ContextLayer>& context,
							  RunHelper& run_helper) {
	bool is_selected = false;
	SeedExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_histories.size() == 1
				&& run_helper.experiment_histories.back()->experiment == this) {
			history = (SeedExperimentHistory*)run_helper.experiment_histories.back();
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
					history = new SeedExperimentHistory(this);
					run_helper.experiment_histories.push_back(history);
					is_selected = true;
				}

				run_helper.experiments_seen_order.push_back(this);
			}
		}
	}

	if (is_selected) {
		switch (this->state) {
		case SEED_EXPERIMENT_STATE_MEASURE_EXISTING:
			measure_existing_activate(context,
									  history);
			break;
		case SEED_EXPERIMENT_STATE_EXPLORE_SEED:
			explore_seed_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  history);
			break;
		case SEED_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_activate(curr_node,
							   problem,
							   context,
							   run_helper,
							   history);
			break;
		case SEED_EXPERIMENT_STATE_EXPLORE_BACK:
			explore_back_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  history);
			break;
		case SEED_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node,
							 problem,
							 context,
							 run_helper,
							 history);
			break;
		case SEED_EXPERIMENT_STATE_VERIFY_EXISTING:
			verify_existing_activate(context,
									 history);
			break;
		case SEED_EXPERIMENT_STATE_VERIFY:
			verify_activate(curr_node,
							problem,
							context,
							run_helper,
							history);
			break;
		#if defined(MDEBUG) && MDEBUG
		case SEED_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(curr_node,
									problem,
									context,
									run_helper);
		#endif /* MDEBUG */
		}

		return true;
	} else {
		return false;
	}
}

void SeedExperiment::back_activate(vector<ContextLayer>& context,
								   RunHelper& run_helper) {
	switch (this->state) {
	case SEED_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_back_activate(context,
									   run_helper);
		break;
	case SEED_EXPERIMENT_STATE_EXPLORE_SEED:
		explore_seed_back_activate(context,
								   run_helper);
		break;
	case SEED_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_back_activate(context,
								run_helper);
		break;
	case SEED_EXPERIMENT_STATE_EXPLORE_BACK:
		explore_back_back_activate(context,
								   run_helper);
		break;
	case SEED_EXPERIMENT_STATE_MEASURE:
		measure_back_activate(context,
							  run_helper);
		break;
	case SEED_EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_back_activate(context,
									  run_helper);
		break;
	case SEED_EXPERIMENT_STATE_VERIFY:
		verify_back_activate(context,
							 run_helper);
		break;
	}
}

void SeedExperiment::backprop(double target_val,
							  RunHelper& run_helper) {
	switch (this->state) {
	case SEED_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper);
		break;
	case SEED_EXPERIMENT_STATE_EXPLORE_SEED:
		explore_seed_backprop(target_val,
							  run_helper);
		break;
	case SEED_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run_helper);
		break;
	case SEED_EXPERIMENT_STATE_EXPLORE_BACK:
		explore_back_backprop(target_val,
							  run_helper);
		break;
	case SEED_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	case SEED_EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case SEED_EXPERIMENT_STATE_VERIFY:
		verify_backprop(target_val,
						run_helper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case SEED_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
