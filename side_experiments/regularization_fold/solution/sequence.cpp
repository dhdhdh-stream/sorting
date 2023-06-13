#include "sequence.h"

using namespace std;



void Sequence::activate(int experiment_step_index,
						vector<double>& flat_vals,
						vector<vector<double>>& input_vals,
						vector<vector<StateDefinition*>>& input_types,
						RunHelper& run_helper,
						SequenceHistory* history) {
	vector<ContextLayer> sequence_context;
	for (int r_index = 0; r_index < (int)this->rising_scopes.size()-1; r_index++) {
		sequence_context.push_back(ContextLayer(this->rising_scopes[r_index]->id,
												this->rising_node_ids[r_index][0],
												&input_vals[r_index],
												&input_types[r_index],
												NULL));	// can be NULL as there won't be experiment inner
	}

	for (int r_index = (int)this->rising_scopes.size()-1; r_index >= 0; r_index--) {
		run_helper.scope_state_networks = this->rising_state_networks[r_index];
		run_helper.scope_score_networks = this->rising_score_networks[r_index];

		for (int n_index = 0; n_index < (int)this->rising_node_ids[r_index].size(); n_index++) {
			if (this->rising_scopes[r_index]->nodes[this->rising_node_ids[n_index]]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->rising_scopes[r_index]->nodes[this->rising_node_ids[n_index]];
				ActionNodeHistory* action_node_history = new ActionNodeHistory(action_node);
				// TODO: add to history somehow
				action_node->activate(flat_vals,
									  input_vals[r_index],
									  input_types[r_index],
									  run_helper,
									  action_node_history);

			} else {
				// NODE_TYPE_INNER_SCOPE

			}
		}
	}
}
