#include "signal_helpers.h"

#include "branch_node.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void fetch_input_helper(ScopeHistory* scope_history,
						SignalInput& input,
						vector<int>& explore_index,
						int l_index,
						double& val,
						bool& is_on) {
	map<int, AbstractNodeHistory*>::iterator it = scope_history
		->node_histories.find(input.node_context[l_index]);
	if (it != scope_history->node_histories.end()) {
		if (l_index >= (int)explore_index.size()
				|| (input.is_pre && it->second->index <= explore_index[l_index])
				|| (!input.is_pre && it->second->index > explore_index[l_index])) {
			if (l_index == (int)input.scope_context.size()-1) {
				switch (it->second->node->type) {
				case NODE_TYPE_BRANCH:
					{
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							val = 1.0;
							is_on = true;
						} else {
							val = -1.0;
							is_on = true;
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
						val = obs_node_history->obs_history[input.obs_index];
						is_on = true;
					}
					break;
				}
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				fetch_input_helper(scope_node_history->scope_history,
								   input,
								   explore_index,
								   l_index+1,
								   val,
								   is_on);
			}
		} else {
			val = 0.0;
			is_on = false;
		}
	} else {
		val = 0.0;
		is_on = false;
	}
}
