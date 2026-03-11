#include "helpers.h"

#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void iter_update_vals_helper(ScopeHistory* scope_history,
							 double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
			iter_update_vals_helper(scope_node_history->scope_history,
									target_val);
		}

		h_it->second->node->sum_vals += target_val;
		h_it->second->node->val_count++;
	}
}

void update_vals(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->val_count == 0) {
				it->second->val_average = 0.0;
			} else {
				it->second->val_average = it->second->sum_vals / it->second->val_count;

				it->second->sum_vals = 0.0;
				it->second->val_count = 0;
			}
		}
	}
}

void iter_update_damage_helper(ScopeHistory* scope_history,
							   double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				iter_update_damage_helper(scope_node_history->scope_history,
										  target_val);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)h_it->second;
				if (obs_node_history->is_damage) {
					h_it->second->node->sum_start_damage += target_val;
					h_it->second->node->start_damage_count++;

					if (obs_node_history->end_damage != NULL) {
						obs_node_history->end_damage->sum_end_damage += target_val;
						obs_node_history->end_damage->end_damage_count++;
					}
				}
			}
			break;
		}
	}
}

void update_damage(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->start_damage_count == 0) {
				it->second->start_damage = 0.0;
			} else {
				double val_average = it->second->sum_start_damage / it->second->start_damage_count;
				it->second->start_damage = val_average - it->second->val_average;

				it->second->sum_start_damage = 0.0;
				it->second->start_damage_count;
			}

			if (it->second->end_damage_count == 0) {
				it->second->end_damage = 0.0;
			} else {
				double val_average = it->second->sum_end_damage / it->second->end_damage_count;
				it->second->end_damage = val_average - it->second->val_average;

				it->second->sum_end_damage = 0.0;
				it->second->end_damage_count;
			}
		}
	}
}
