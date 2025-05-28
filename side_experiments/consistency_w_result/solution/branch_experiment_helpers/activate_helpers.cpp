#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void BranchExperiment::result_activate(AbstractNode* experiment_node,
									   bool is_branch,
									   AbstractNode*& curr_node,
									   Problem* problem,
									   RunHelper& run_helper,
									   ScopeHistory* scope_history) {
	BranchExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_history == NULL) {
			history = new BranchExperimentHistory(this);
			run_helper.experiment_history = history;
		}
	}
}

void BranchExperiment::activate(AbstractNode* experiment_node,
								bool is_branch,
								AbstractNode*& curr_node,
								Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	bool is_selected = false;
	BranchExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_history != NULL) {
			history = (BranchExperimentHistory*)run_helper.experiment_history;
			is_selected = true;
		} else if (run_helper.experiment_history == NULL) {
			history = new BranchExperimentHistory(this);
			run_helper.experiment_history = history;
			is_selected = true;
		}
	}

	if (is_selected) {
		switch (this->state) {
		case BRANCH_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 run_helper,
							 scope_history,
							 history);
			break;
		case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
			new_gather_activate(scope_history);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_activate(curr_node,
							   problem,
							   run_helper,
							   scope_history,
							   history);
			break;
		case BRANCH_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node,
							 problem,
							 run_helper,
							 scope_history);
			break;
		#if defined(MDEBUG) && MDEBUG
		case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(curr_node,
									problem,
									run_helper,
									scope_history);
			break;
		#endif /* MDEBUG */
		}
	}
}

void BranchExperiment::back_activate(RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN_NEW) {
		if (scope_history->has_local_experiment) {
			this->match_histories.push_back(
				run_helper.num_matches - scope_history->experiment_num_matches);
		}
	}
}

void BranchExperiment::backprop(double target_val,
								RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_history;
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper,
						 history);
		break;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_backprop();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run_helper,
						   history);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
