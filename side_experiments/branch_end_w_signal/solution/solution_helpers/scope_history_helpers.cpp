#include "solution_helpers.h"

#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void add_existing_samples_helper(ScopeHistory* scope_history,
								 vector<ScopeHistory*>& scope_histories,
								 double target_val,
								 map<Scope*, pair<int,pair<ScopeHistory*,double>>>& to_add) {
	Scope* scope = scope_history->scope;

	map<Scope*, pair<int,pair<ScopeHistory*,double>>>::iterator it = to_add.find(scope);
	if (it == to_add.end()) {
		it = to_add.insert({scope, {0, {NULL, 0.0}}}).first;
	}
	uniform_int_distribution<int> add_distribution(0, it->second.first);
	if (add_distribution(generator) == 0) {
		double sum_vals = target_val;
		int sum_counts = 1;
		for (int l_index = 0; l_index < (int)scope_histories.size(); l_index++) {
			if (scope_histories[l_index]->scope->pre_network != NULL) {
				sum_vals += scope_histories[l_index]->post_val;
				sum_counts++;
			}
		}
		double average_val = sum_vals / sum_counts;
		it->second.second = {scope_history, average_val};
	}
	it->second.first++;

	if (scope->pre_network != NULL) {
		vector<double> inputs = scope_history->pre_obs;
		inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

		scope->post_network->activate(inputs);
		scope_history->post_val = scope->post_network->output->acti_vals[0];
	}

	scope_histories.push_back(scope_history);

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			add_existing_samples_helper(scope_node_history->scope_history,
										scope_histories,
										target_val,
										to_add);
		}
	}

	scope_histories.pop_back();
}

void add_existing_samples(ScopeHistory* scope_history,
						  double target_val) {
	vector<ScopeHistory*> scope_histories;
	map<Scope*, pair<int,pair<ScopeHistory*,double>>> to_add;
	add_existing_samples_helper(scope_history,
								scope_histories,
								target_val,
								to_add);

	for (map<Scope*, pair<int,pair<ScopeHistory*,double>>>::iterator it = to_add.begin();
			it != to_add.end(); it++) {
		it->first->existing_pre_obs.back().push_back(it->second.second.first->pre_obs);
		it->first->existing_post_obs.back().push_back(it->second.second.first->post_obs);
		it->first->existing_target_vals.back().push_back(it->second.second.second);
	}
}
