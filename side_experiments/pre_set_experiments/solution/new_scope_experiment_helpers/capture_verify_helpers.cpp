#if defined(MDEBUG) && MDEBUG

#include <iostream>

#include "new_scope_experiment.h"

#include "constants.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewScopeExperiment::capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewScopeExperimentHistory* history) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	context.back().node_id = -1;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
	this->new_scope->new_scope_capture_verify_activate(
		problem,
		context,
		run_helper,
		inner_scope_history);
	delete inner_scope_history;

	curr_node = this->exit_next_node;
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