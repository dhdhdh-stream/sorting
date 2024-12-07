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

void BranchExperiment::activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_history;

	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_activate(curr_node,
						 problem,
						 context,
						 run_helper,
						 scope_history,
						 history);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   scope_history,
						   history);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_activate(curr_node,
						 problem,
						 context,
						 run_helper,
						 scope_history,
						 history);
		break;
	#if defined(MDEBUG) && MDEBUG
	case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_activate(curr_node,
								problem,
								context,
								run_helper,
								scope_history);
		break;
	#endif /* MDEBUG */
	}
}

void BranchExperiment::backprop(double target_val,
								RunHelper& run_helper) {
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run_helper);
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
