// #if defined(MDEBUG) && MDEBUG

// #include <iostream>

// #include "new_action_experiment.h"

// #include "constants.h"
// #include "problem.h"
// #include "scope.h"
// #include "scope_node.h"

// using namespace std;

// void NewActionExperiment::capture_verify_activate(
// 		int location_index,
// 		AbstractNode*& curr_node,
// 		Problem* problem,
// 		vector<ContextLayer>& context,
// 		RunHelper& run_helper,
// 		NewActionExperimentHistory* history) {
// 	if (this->verify_problems[this->state_iter] == NULL) {
// 		this->verify_problems[this->state_iter] = problem->copy_and_reset();
// 	}
// 	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

// 	curr_node = this->successful_scope_nodes[location_index];
// 	while (true) {
// 		ScopeNode* curr_scope_node = (ScopeNode*)curr_node;

// 		curr_scope_node->new_action_capture_verify_activate(
// 			curr_node,
// 			problem,
// 			context,
// 			run_helper);

// 		if (run_helper.exceeded_limit
// 				|| curr_node == NULL
// 				|| this->scope_context->nodes.find(curr_node->id) != this->scope_context->nodes.end()) {
// 			break;
// 		}
// 	}
// }

// void NewActionExperiment::capture_verify_backprop() {
// 	if (this->verify_problems[this->state_iter] != NULL) {
// 		this->state_iter++;
// 		if (this->state_iter >= NUM_VERIFY_SAMPLES) {
// 			this->result = EXPERIMENT_RESULT_SUCCESS;
// 		}
// 	}
// }

// #endif /* MDEBUG */