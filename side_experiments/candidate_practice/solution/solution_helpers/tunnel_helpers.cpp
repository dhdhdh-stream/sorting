#include "solution_helpers.h"

#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "tunnel.h"

using namespace std;

void set_tunnel(SolutionWrapper* wrapper) {
	vector<pair<Scope*,int>> possible_tunnels;
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (int t_index = 0; t_index < (int)scope->tunnels.size(); t_index++) {
			possible_tunnels.push_back({scope, t_index});
		}
	}

	uniform_int_distribution<int> existing_distribution(0, 1);
	if (possible_tunnels.size() != 0
			&& existing_distribution(generator) == 0) {
		uniform_int_distribution<int> distribution(0, possible_tunnels.size()-1);
		int index = distribution(generator);
		wrapper->curr_tunnel_parent = possible_tunnels[index].first;
		wrapper->curr_tunnel_index = possible_tunnels[index].second;
	} else {
		wrapper->curr_tunnel_parent = NULL;
	}
}

void update_tunnel_try_history(SolutionWrapper* wrapper) {
	if (wrapper->curr_tunnel_parent != NULL) {
		Scope* solution_scope = wrapper->solution->scopes[wrapper->curr_tunnel_parent->id];
		if ((int)solution_scope->tunnels.size() > wrapper->curr_tunnel_index) {
			vector<double> vals(wrapper->curr_solution->existing_scope_histories.size());
			for (int h_index = 0; h_index < (int)wrapper->curr_solution->existing_scope_histories.size(); h_index++) {
				double sum_vals = 0.0;
				measure_tunnel_vals_helper(wrapper->curr_solution->existing_scope_histories[h_index],
										   wrapper->curr_tunnel_parent,
										   wrapper->curr_tunnel_index,
										   sum_vals);
				vals[h_index] = sum_vals;
			}

			double sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
				sum_vals += vals[h_index];
			}
			double val_average = sum_vals / (double)vals.size();

			Tunnel* solution_tunnel = solution_scope->tunnels[wrapper->curr_tunnel_index];
			Tunnel* potential_solution_tunnel = wrapper->curr_tunnel_parent->tunnels[wrapper->curr_tunnel_index];
			Tunnel* best_solution_tunnel;
			if (wrapper->best_solution == NULL) {
				best_solution_tunnel = NULL;
			} else {
				Scope* best_solution_scope = wrapper->best_solution->scopes[wrapper->curr_tunnel_parent->id];
				best_solution_tunnel = best_solution_scope->tunnels[wrapper->curr_tunnel_index];
			}

			int new_status;
			if (val_average <= solution_tunnel->val_history.back()) {
				new_status = TUNNEL_TRY_STATUS_SIGNAL_FAIL;
			} else {
				if (wrapper->curr_solution->curr_score <= wrapper->solution->curr_score) {
					new_status = TUNNEL_TRY_STATUS_TRUE_FAIL;
				} else {
					new_status = TUNNEL_TRY_STATUS_TRUE_SUCCESS;
				}
			}

			solution_tunnel->try_history.push_back(new_status);
			potential_solution_tunnel->try_history.push_back(new_status);
			if (best_solution_tunnel != NULL) {
				best_solution_tunnel->try_history.push_back(new_status);
			}
		}
	}
}

void measure_tunnel_vals_helper(ScopeHistory* scope_history,
								Scope* tunnel_parent,
								int tunnel_index,
								double& sum_vals) {
	Scope* scope = scope_history->scope;

	if (scope == tunnel_parent) {
		if (!scope_history->tunnel_is_init[tunnel_index]) {
			scope_history->tunnel_is_init[tunnel_index] = true;
			Tunnel* tunnel = scope->tunnels[tunnel_index];
			scope_history->tunnel_vals[tunnel_index] = tunnel->get_signal(scope_history->obs_history);
		}
		sum_vals += scope_history->tunnel_vals[tunnel_index];
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == tunnel_parent) {
				is_child = true;
				break;
			}
		}
		if (is_child) {
			for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
					it != scope_history->node_histories.end(); it++) {
				if (it->second->node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					measure_tunnel_vals_helper(scope_node_history->scope_history,
											   tunnel_parent,
											   tunnel_index,
											   sum_vals);
				}
			}
		}
	}
}

void measure_tunnel_vals_helper(ScopeHistory* scope_history) {
	Scope* scope = scope_history->scope;

	for (int t_index = 0; t_index < (int)scope->tunnels.size(); t_index++) {
		if (!scope_history->tunnel_is_init[t_index]) {
			scope_history->tunnel_is_init[t_index] = true;
			scope_history->tunnel_vals[t_index] = scope->tunnels[t_index]->get_signal(scope_history->obs_history);
		}
		scope->tunnels[t_index]->vals.push_back(scope_history->tunnel_vals[t_index]);
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			measure_tunnel_vals_helper(scope_node_history->scope_history);
		}
	}
}
