#include "fetch_helper.h"

using namespace std;

void fetch_helper(vector<Object>& fetch_obj_vals,
				  vector<vector<int>>& fetch_dependencies,
				  ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	vector<Object> fetch_obj_vals_snapshot = fetch_obj_vals;

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;

				for (int o_index = 0; o_index < (int)fetch_obj_vals.size(); o_index++) {
					ObjectDefinition* curr_definition = obj_vals[o_index].definition;
					while (true) {
						map<ObjectDefinition*, vector<ObjectNetwork*>>::iterator it = action_node->networks.find(curr_definition);
						if (it != action_node->networks.end()) {
							for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
								ObjectNetwork* object_network = it->second[n_index];
								Network* network = object_network->network;

								network->obs_input->acti_vals[0] = action_node_history->obs_snapshot;

								for (int s_index = 0; s_index < network->state_size; s_index++) {
									if (fetch_dependencies[o_index][object_network->object_dependency_index[s_index]] == -1) {
										network->state_input->acti_vals[s_index] = action_node_history->obj_vals_snapshot[object_network->scope_object_index[s_index]]
											->state_vals[object_network->state_index[s_index]];
									} else {
										network->state_input->acti_vals[s_index] = fetch_obj_vals_snapshot[
											fetch_dependencies[o_index][object_network->object_dependency_index[s_index]]]->state_vals[object_network->state_index[s_index]];
									}
								}

								network->activate();

								fetch_obj_vals[o_index]->state_vals[object_network->target_index] += network->output->acti_vals[0];
							}
						}

						curr_definition = curr_definition->parent;
						if (curr_definition == NULL) {
							break;
						}
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				fetch_helper(fetch_obj_vals,
							 fetch_dependencies,
							 scope_node_history->inner_scope_history);
			}
		}
	}
}
