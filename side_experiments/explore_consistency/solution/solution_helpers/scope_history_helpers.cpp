#include "solution_helpers.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void add_existing_samples_helper(ScopeHistory* scope_history) {
	Scope* scope = scope_history->scope;

	if ((int)scope->existing_pre_obs.size() < MAX_SAMPLES) {
		scope->existing_pre_obs.push_back(scope_history->pre_obs);
		scope->existing_post_obs.push_back(scope_history->post_obs);
	} else {
		uniform_int_distribution<int> distribution(0, scope->existing_pre_obs.size()-1);
		int index = distribution(generator);
		scope->existing_pre_obs[index] = scope_history->pre_obs;
		scope->existing_post_obs[index] = scope_history->post_obs;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			add_existing_samples_helper(scope_node_history->scope_history);
		}
	}
}
