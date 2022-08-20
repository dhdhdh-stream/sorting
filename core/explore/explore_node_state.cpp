#include "explore_node_state.h"

using namespace std;

void ExploreNodeState::process() override {
	SolutionNode* scope = this->explore->solution->nodes[this->scope_index];

	if (scope->type == NODE_TYPE_IF_START) {
		SolutionNodeIfStart* scope_if_start = (SolutionNodeIfStart*)scope;
		scope_if_start->scope_states_on.insert(scope_if_start->scope_states_on.end(),
			this->new_state_indexes.begin(), this->new_state_indexes.end());
	} else if (scope->type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* scope_loop_start = (SolutionNodeLoopStart*)scope;
		scope_loop_start->scope_states_on.insert(scope_loop_start->scope_states_on.end(),
			this->new_state_indexes.begin(), this->new_state_indexes.end());
	}
}
