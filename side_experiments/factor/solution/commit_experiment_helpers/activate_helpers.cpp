#include "commit_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void CommitExperiment::activate(AbstractNode* experiment_node,
								bool is_branch,
								AbstractNode*& curr_node,
								Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	bool is_selected = false;
	CommitExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_history != NULL
				&& run_helper.experiment_history->experiment == this) {
			history = (CommitExperimentHistory*)run_helper.experiment_history;
			is_selected = true;
		} else if (run_helper.experiment_history == NULL) {
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
					history = new CommitExperimentHistory(this);
					run_helper.experiment_history = history;
					is_selected = true;
				}

				run_helper.experiments_seen_order.push_back(this);
			}
		}
	}

	if (is_selected) {
		switch (this->state) {
		case COMMIT_EXPERIMENT_STATE_EXISTING_GATHER:
			existing_gather_activate(scope_history);
			break;
		case COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_activate(run_helper,
									scope_history,
									history);
			break;
		case COMMIT_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 run_helper,
							 scope_history,
							 history);
			break;
		case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
			find_save_activate(curr_node,
							   problem,
							   run_helper);
			break;
		case COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER:
			commit_existing_gather_activate(curr_node,
											problem,
											run_helper,
											scope_history);
			break;
		case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
			commit_train_existing_activate(curr_node,
										   problem,
										   run_helper,
										   scope_history,
										   history);
			break;
		case COMMIT_EXPERIMENT_STATE_COMMIT_NEW_GATHER:
			commit_new_gather_activate(curr_node,
									   problem,
									   run_helper,
									   scope_history);
			break;
		case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW:
			commit_train_new_activate(curr_node,
									  problem,
									  run_helper,
									  scope_history,
									  history);
			break;
		#if defined(MDEBUG) && MDEBUG
		case COMMIT_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(curr_node,
									problem,
									run_helper,
									scope_history);
			break;
		#endif /* MDEBUG */
		}
	}
}

void CommitExperiment::backprop(double target_val,
								RunHelper& run_helper) {
	CommitExperimentHistory* history = (CommitExperimentHistory*)run_helper.experiment_history;
	switch (this->state) {
	case COMMIT_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_backprop();
		break;
	case COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								run_helper,
								history);
		break;
	case COMMIT_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper,
						 history);
		break;
	case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
		find_save_backprop(target_val,
						   run_helper);
		break;
	case COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER:
		commit_existing_gather_backprop();
		break;
	case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
		commit_train_existing_backprop(target_val,
									   run_helper,
									   history);
		break;
	case COMMIT_EXPERIMENT_STATE_COMMIT_NEW_GATHER:
		commit_new_gather_backprop();
		break;
	case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW:
		commit_train_new_backprop(target_val,
								  run_helper,
								  history);
		break;
	#if defined(MDEBUG) && MDEBUG
	case COMMIT_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
