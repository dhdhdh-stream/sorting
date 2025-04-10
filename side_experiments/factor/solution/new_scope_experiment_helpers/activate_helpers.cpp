#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewScopeExperiment::pre_activate(RunHelper& run_helper) {
	if (run_helper.experiment_history == NULL) {
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
				run_helper.experiment_history = new NewScopeExperimentHistory(this);
			}

			run_helper.experiments_seen_order.push_back(this);
		}
	}
}

void NewScopeExperiment::activate(AbstractNode* experiment_node,
								  bool is_branch,
								  AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	if (run_helper.experiment_history != NULL
			&& run_helper.experiment_history->experiment == this) {
		bool has_match = false;
		bool is_test;
		int location_index;
		if (this->test_location_start == experiment_node
				&& this->test_location_is_branch == is_branch) {
			has_match = true;
			is_test = true;
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

		if (has_match) {
			NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_history;

			switch (this->state) {
			case NEW_SCOPE_EXPERIMENT_STATE_EXPLORE:
				if (is_test) {
					test_activate(curr_node,
								  problem,
								  run_helper,
								  history);
				} else {
					this->successful_scope_nodes[location_index]->experiment_activate(
						curr_node,
						problem,
						run_helper,
						scope_history);
				}
				break;
			#if defined(MDEBUG) && MDEBUG
			case NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY:
				capture_verify_activate(location_index,
										curr_node,
										problem,
										run_helper,
										scope_history);
				break;
			#endif /* MDEBUG */
			}
		}
	}
}

void NewScopeExperiment::backprop(double target_val,
								  RunHelper& run_helper) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_history;

	switch (this->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_EXPLORE:
		if (history->hit_test) {
			test_backprop(target_val,
						  run_helper,
						  history);
		}
		break;
	#if defined(MDEBUG) && MDEBUG
	case NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
