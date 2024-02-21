#include "seed_experiment_filter.h"

using namespace std;

void SeedExperimentFilter::filter_activate(AbstractNode*& curr_node,
										   Problem* problem,
										   vector<ContextLayer>& context,
										   int& exit_depth,
										   AbstractNode*& exit_node,
										   RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->actions[s_index]);
			this->actions[s_index]->activate(curr_node,
											 problem,
											 context,
											 exit_depth,
											 exit_node,
											 run_helper,
											 action_node_history);
			delete action_node_history;
		} else if (this->step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->existing_scopes[s_index]);
			this->existing_scopes[s_index]->activate(curr_node,
													 problem,
													 context,
													 exit_depth,
													 exit_node,
													 run_helper,
													 scope_node_history);
			delete scope_node_history;
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->potential_scopes[s_index]);
			this->potential_scopes[s_index]->activate(curr_node,
													  problem,
													  context,
													  exit_depth,
													  exit_node,
													  run_helper,
													  scope_node_history);
			delete scope_node_history;
		}
	}

	if (this->exit_depth == 0) {
		curr_node = this->exit_next_node;
	} else {
		exit_depth = this->exit_depth-1;
		exit_node = this->exit_next_node;
	}
}
