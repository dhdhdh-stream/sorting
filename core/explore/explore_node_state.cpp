#include "explore_node_state.h"

using namespace std;

void ExploreNodeState::process(Solution* solution) override {
	if (this->scope->type == NODE_TYPE_GOTO_START) {
		SolutionNodeGotoStart* scope_goto_start = (SolutionNodeGotoStart*)this->scope;
		scope_goto_start->states_on->push_back(this->new_state_index);
	} else if (this->scope->type == NODE_TYPE_IF_START) {
		SolutionNodeIfStart* scope_if_start = (SolutionNodeIfStart*)this->scope;
		scope_if_start->states_on->push_back(this->new_state_index);
	} else if (this->scope->type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* scope_loop_start = (SolutionNodeLoopStart*)this->scope;
		scope_loop_start->states_on->push_back(this->new_state_index);
	}
}
