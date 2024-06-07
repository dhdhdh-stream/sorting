#if defined(MDEBUG) && MDEBUG

#include <iostream>

#include "new_action_experiment.h"

#include "constants.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void NewActionExperiment::capture_verify_activate(
		int location_index,
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewActionExperimentHistory* history) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	this->scope_context->new_action_capture_verify_activate(
		this->starting_node,
		this->included_nodes,
		problem,
		context,
		run_helper);

	run_helper.num_actions += 2;

	curr_node = this->successful_location_exits[location_index];
}

void NewActionExperiment::capture_verify_backprop() {
	if (this->verify_problems[this->state_iter] != NULL) {
		this->state_iter++;
		if (this->state_iter >= NUM_VERIFY_SAMPLES) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		}
	}
}

#endif /* MDEBUG */