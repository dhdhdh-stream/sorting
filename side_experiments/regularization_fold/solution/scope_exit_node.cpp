#include "scope_exit_node.h"

using namespace std;



void ScopeExitNode::activate(vector<double>& state_vals,
							 vector<StateDefinition*>& state_types,
							 vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeExitNodeHistory* history) {
	history->state_vals_snapshot = vector<vector<double>>(this->exit_depth+1);
	for (int l_index = 0; l_index < this->exit_depth; l_index++) {
		history->state_vals_snapshot[l_index] = *(context[
			context.size() - this->exit_depth + l_index].state_vals);
	}
	history->state_vals_snapshot[this->exit_depth-1] = state_vals;

	vector<double>* outer_state_vals = context[context.size() - this->exit_depth].state_vals;
	vector<StateDefinition*>* outer_state_types = context[context.size() - this->exit_depth].state_types;

	for (int s_index = 0; s_index < (int)this->networks.size(); s_index++) {
		if (outer_state_types->at(s_index) != NULL) {
			map<StateDefinition*, ExitNetwork*>::iterator it = this->networks[s_index].find(outer_state_types->at(s_index));
			if (it != this->networks[s_index].end()
					&& it->second != NULL) {
				
			}
		}
	}
}
