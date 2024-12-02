#include "new_scope_experiment.h"

#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewScopeExperiment::activate(AbstractNode*& curr_node,
								  Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_history;

	switch (this->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE:
	case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_1ST:
	case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_2ND:
		test_activate(curr_node,
					  problem,
					  context,
					  run_helper,
					  history);
		break;
	#if defined(MDEBUG) && MDEBUG
	case NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_activate(curr_node,
								problem,
								context,
								run_helper,
								history);
		break;
	#endif /* MDEBUG */
	}
}

void NewScopeExperiment::backprop(double target_val,
								  RunHelper& run_helper) {
	switch (this->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE:
	case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_1ST:
	case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_2ND:
		test_backprop(target_val,
					  run_helper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
