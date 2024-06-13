#include "new_action_experiment.h"

#include <iostream>

#include "scope.h"
#include "scope_node.h"

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

	curr_node = this->successful_scope_nodes[location_index];
}
