#include "solution_helpers.h"

#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void add_existing_samples_helper(ScopeHistory* scope_history) {
	Scope* scope = scope_history->scope;

	if (scope->signal_status != SIGNAL_STATUS_FAIL) {
		int max_sample_per_timestamp = (TOTAL_MAX_SAMPLES + (int)scope->existing_pre_obs.size() - 1) / (int)scope->existing_pre_obs.size();
		if ((int)scope->existing_pre_obs.back().size() < max_sample_per_timestamp) {
			scope->existing_pre_obs.back().push_back(scope_history->pre_obs);
			scope->existing_post_obs.back().push_back(scope_history->post_obs);
		} else {
			uniform_int_distribution<int> distribution(0, scope->existing_pre_obs.back().size()-1);
			int index = distribution(generator);
			scope->existing_pre_obs.back()[index] = scope_history->pre_obs;
			scope->existing_post_obs.back()[index] = scope_history->post_obs;
		}
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			add_existing_samples_helper(scope_node_history->scope_history);
		}
	}
}
