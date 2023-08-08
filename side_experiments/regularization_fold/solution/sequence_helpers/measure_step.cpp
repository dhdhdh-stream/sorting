#include "sequence.h"

#include "abstract_node.h"
#include "action_node.h"
#include "layer.h"
#include "scope.h"
#include "scope_node.h"
#include "state_network.h"

using namespace std;

void Sequence::measure_pre_activate_helper(vector<double>& input_vals,
										   ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.find(scope_id);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->node->id;

				if (it != this->state_networks.end()
						&& node_id < (int)it->second.size()
						&& it->second[node_id].size() != 0) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

					for (int ii_index = 0; ii_index < (int)this->input_types.size(); ii_index++) {
						StateNetwork* network = it->second[node_id][ii_index];
						network->new_activate(action_node_history->obs_snapshot,
											  action_node_history->starting_state_vals_snapshot,
											  action_node_history->starting_new_state_vals_snapshot,
											  input_vals[ii_index]);
						input_vals[ii_index] += network->output->acti_vals[0];
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];

				measure_pre_activate_helper(input_vals,
											scope_node_history->inner_scope_history);
			}
		}
	}
}

void Sequence::measure_activate_pull(vector<double>& input_vals,
									 vector<ForwardContextLayer>& context) {
	measure_pre_activate_helper(input_vals,
								context[0].scope_history);

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		// this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL
		input_vals[i_index] += context[context.size()-1 - this->input_local_scope_depths[i_index]]
			.state_vals->at(this->input_local_input_indexes[i_index]);
	}
}

void Sequence::measure_activate_reset(vector<double>& input_vals,
									  vector<ForwardContextLayer>& context) {
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		// this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL
		context[context.size()-1 - this->input_local_scope_depths[i_index]]
			.state_vals->at(this->input_local_input_indexes[i_index]) = input_vals[i_index];
	}
}
