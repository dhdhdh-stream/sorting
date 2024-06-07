#include "new_action_experiment.h"

#include "scope.h"

using namespace std;

void NewActionExperiment::successful_activate(
		int location_index,
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewActionExperimentHistory* history) {
	if (history->test_location_index == -1) {
		context.back().scope_history->callback_experiment_history = history;
	}

	this->scope_context->new_action_activate(this->starting_node,
											 this->included_nodes,
											 problem,
											 context,
											 run_helper);

	/**
	 * - increment properly mainly for MDEBUG
	 */
	run_helper.num_actions += 2;

	curr_node = this->successful_location_exits[location_index];
}
