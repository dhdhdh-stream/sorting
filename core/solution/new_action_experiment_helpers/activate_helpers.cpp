#include "new_action_experiment.h"

#include <iostream>

#include "globals.h"
#include "scope.h"

using namespace std;

bool NewActionExperiment::activate(AbstractNode* experiment_node,
								   bool is_branch,
								   AbstractNode*& curr_node,
								   Problem* problem,
								   vector<ContextLayer>& context,
								   RunHelper& run_helper) {
	bool has_match = false;
	bool is_test;
	int location_index;
	if (run_helper.experiment_scope_history != NULL) {
		for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
			if (this->test_location_starts[t_index] == experiment_node
					&& this->test_location_is_branch[t_index] == is_branch) {
				has_match = true;
				is_test = true;
				location_index = t_index;
				break;
			}
		}
		if (!has_match) {
			for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
				if (this->successful_location_starts[s_index] == experiment_node
						&& this->successful_location_is_branch[s_index] == is_branch) {
					has_match = true;
					is_test = false;
					location_index = s_index;
					break;
				}
			}
		}
	}

	bool is_selected = false;
	NewActionExperimentHistory* history = NULL;
	if (has_match) {
		if (run_helper.experiment_scope_history->experiment_histories.size() == 1
				&& run_helper.experiment_scope_history->experiment_histories[0]->experiment == this) {
			history = (NewActionExperimentHistory*)run_helper.experiment_scope_history->experiment_histories[0];
			is_selected = true;
		} else if (run_helper.experiment_scope_history->scope->id == this->scope_context->id
				&& run_helper.experiment_scope_history->experiment_histories.size() == 0) {
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
					history = new NewActionExperimentHistory(this);
					run_helper.experiment_scope_history->experiment_histories.push_back(history);
					is_selected = true;
				}

				run_helper.experiment_scope_history->experiments_seen_order.push_back(this);
			}
		}
	}

	if (is_selected) {
		switch (this->state) {
		case NEW_ACTION_EXPERIMENT_STATE_EXPLORE:
			if (is_test) {
				if (history->test_location_index == -1
						|| history->test_location_index == location_index) {
					test_activate(location_index,
								  curr_node,
								  problem,
								  context,
								  run_helper,
								  history);

					return true;
				} else {
					return false;
				}
			} else {
				successful_activate(location_index,
									curr_node,
									problem,
									context,
									run_helper,
									history);
				return true;
			}
		#if defined(MDEBUG) && MDEBUG
		case NEW_ACTION_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(location_index,
									curr_node,
									problem,
									context,
									run_helper,
									history);
			return true;
		#endif /* MDEBUG */
		}
	}

	return false;
}

void NewActionExperiment::backprop(EvalHistory* eval_history,
								   Problem* problem,
								   vector<ContextLayer>& context,
								   RunHelper& run_helper) {
	NewActionExperimentHistory* history = (NewActionExperimentHistory*)run_helper.experiment_scope_history->experiment_histories.back();

	switch (this->state) {
	case NEW_ACTION_EXPERIMENT_STATE_EXPLORE:
		if (history->test_location_index == -1) {
			add_new_test_location(context.back().scope_history);
		} else {
			test_backprop(eval_history,
						  problem,
						  context,
						  run_helper);
		}
		break;
	#if defined(MDEBUG) && MDEBUG
	case NEW_ACTION_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}