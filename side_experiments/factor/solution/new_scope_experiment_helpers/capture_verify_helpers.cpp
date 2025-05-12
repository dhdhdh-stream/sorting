#if defined(MDEBUG) && MDEBUG

#include <iostream>

#include "new_scope_experiment.h"

#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewScopeExperiment::capture_verify_activate(
		int location_index,
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
	this->new_scope->new_scope_capture_verify_activate(
		problem,
		run_helper,
		inner_scope_history);
	delete inner_scope_history;

	curr_node = this->successful_obs_nodes[location_index];
}

void NewScopeExperiment::capture_verify_backprop() {
	if (this->verify_problems[this->state_iter] != NULL) {
		this->state_iter++;
		if (this->state_iter >= NUM_VERIFY_SAMPLES) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		}
	}
}

#endif /* MDEBUG */