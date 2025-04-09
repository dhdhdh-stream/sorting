#include "solution_helpers.h"

#include "obs_node.h"
#include "scope.h"

using namespace std;

void fetch_input_helper(ScopeHistory* scope_history,
						Input input,
						double& obs) {
	map<int, AbstractNodeHistory*>::iterator it = scope_history
		->node_histories.find(input.node_context[0]);
	if (it != scope_history->node_histories.end()) {
		obs = 0.0;
	} else {
		ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
		obs = obs_node_history->obs_history[input.obs_index];
	}
}
