#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewScopeExperiment::activate(AbstractNode* experiment_node,
								  bool is_branch,
								  AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	bool is_selected = false;
	NewScopeExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_history != NULL) {
			history = (NewScopeExperimentHistory*)run_helper.experiment_history;
			is_selected = true;
		} else if (run_helper.experiment_history == NULL) {
			history = new NewScopeExperimentHistory(this);
			run_helper.experiment_history = history;
			is_selected = true;
		}
	}

	if (is_selected) {
		switch (this->state) {
		case NEW_SCOPE_EXPERIMENT_STATE_MEASURE:
		case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_1ST:
		case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_2ND:
			test_activate(curr_node,
						  problem,
						  run_helper,
						  history);
			break;
		#if defined(MDEBUG) && MDEBUG
		case NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(curr_node,
									problem,
									run_helper,
									scope_history);
			break;
		#endif /* MDEBUG */
		}
	}
}

void NewScopeExperiment::backprop(double target_val,
								  RunHelper& run_helper) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_history;

	switch (this->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE:
	case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_1ST:
	case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_2ND:
		test_backprop(target_val,
					  run_helper,
					  history);
		break;
	#if defined(MDEBUG) && MDEBUG
	case NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
