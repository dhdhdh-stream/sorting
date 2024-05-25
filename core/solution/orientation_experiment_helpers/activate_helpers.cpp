#include "orientation_experiment.h"

#include "globals.h"
#include "scope.h"

using namespace std;

bool OrientationExperiment::activate(AbstractNode* experiment_node,
									 bool is_branch,
									 AbstractNode*& curr_node,
									 Problem* problem,
									 std::vector<ContextLayer>& context,
									 RunHelper& run_helper) {
	bool is_selected = false;
	OrientationExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_scope_history->experiment_histories.size() == 1
				&& run_helper.experiment_scope_history->experiment_histories[0]->experiment == this) {
			history = (OrientationExperimentHistory*)run_helper.experiment_scope_history->experiment_histories[0];
			is_selected = true;
		} else if (run_helper.experiment_scope_history->experiment_histories.size() == 0) {
			bool has_seen = false;
			for (int e_index = 0; e_index < (int)run_helper.experiment_scope_history->experiments_seen_order.size(); e_index++) {
				if (run_helper.experiment_scope_history->experiments_seen_order[e_index] == this) {
					has_seen = true;
					break;
				}
			}
			if (!has_seen) {
				double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					history = new OrientationExperimentHistory(this);
					run_helper.experiment_scope_history->experiment_histories.push_back(history);
					is_selected = true;
				}

				run_helper.experiment_scope_history->experiments_seen_order.push_back(this);
			}
		}
	}

	if (is_selected) {
		switch (this->state) {
		case ORIENTATION_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_activate(context,
									run_helper);
			return false;
		case ORIENTATION_EXPERIMENT_STATE_EXPLORE_MISGUESS:
			explore_misguess_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history);
			return true;
		case ORIENTATION_EXPERIMENT_STATE_EXPLORE_IMPACT:
			explore_impact_activate(curr_node,
									problem,
									context,
									run_helper);
			return true;
		case ORIENTATION_EXPERIMENT_STATE_MEASURE:
			return measure_activate(curr_node,
									problem,
									context,
									run_helper);
		case ORIENTATION_EXPERIMENT_STATE_VERIFY_1ST:
		case ORIENTATION_EXPERIMENT_STATE_VERIFY_2ND:
			return verify_activate(curr_node,
								   problem,
								   context,
								   run_helper);
		#if defined(MDEBUG) && MDEBUG
		case ORIENTATION_EXPERIMENT_STATE_CAPTURE_VERIFY:
			return capture_verify_activate(curr_node,
										   problem,
										   context,
										   run_helper);
		#endif /* MDEBUG */
		}
	}

	return false;
}

void OrientationExperiment::backprop(EvalHistory* outer_eval_history,
									 EvalHistory* eval_history,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper) {
	switch (this->state) {
	case ORIENTATION_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(outer_eval_history,
								eval_history,
								problem,
								context,
								run_helper);
		break;
	case ORIENTATION_EXPERIMENT_STATE_EXPLORE_MISGUESS:
		explore_misguess_backprop(outer_eval_history,
								  eval_history,
								  problem,
								  context,
								  run_helper);
		break;
	case ORIENTATION_EXPERIMENT_STATE_EXPLORE_IMPACT:
		explore_impact_backprop(outer_eval_history,
								eval_history,
								problem,
								context,
								run_helper);
		break;
	case ORIENTATION_EXPERIMENT_STATE_MEASURE:
		measure_backprop(outer_eval_history,
						 eval_history,
						 problem,
						 context,
						 run_helper);
		break;
	case ORIENTATION_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case ORIENTATION_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_backprop(outer_eval_history,
								 eval_history,
								 problem,
								 context,
								 run_helper);
		break;
	case ORIENTATION_EXPERIMENT_STATE_VERIFY_1ST:
	case ORIENTATION_EXPERIMENT_STATE_VERIFY_2ND:
		verify_backprop(outer_eval_history,
						eval_history,
						problem,
						context,
						run_helper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case ORIENTATION_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
