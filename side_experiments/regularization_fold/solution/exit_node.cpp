#include "scope_exit_node.h"

using namespace std;



void ScopeExitNode::activate(vector<double>& state_vals,
							 vector<StateDefinition*>& state_types,
							 vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeExitNodeHistory* history) {
	history->state_vals_snapshot = vector<vector<double>>(this->exit_depth+1);
	for (int l_index = 0; l_index < this->exit_depth; l_index++) {
		history->state_vals_snapshot[l_index] = *(context[
			context.size() - this->exit_depth + l_index].state_vals);
	}
	history->state_vals_snapshot[this->exit_depth-1] = state_vals;

	vector<double>* outer_state_vals = context[context.size() - this->exit_depth].state_vals;
	vector<StateDefinition*>* outer_state_types = context[context.size() - this->exit_depth].state_types;

	for (int s_index = 0; s_index < (int)this->networks.size(); s_index++) {
		if (outer_state_types->at(s_index) != NULL) {
			map<StateDefinition*, ExitNetwork*>::iterator it = this->networks[s_index].find(outer_state_types->at(s_index));
			if (it != this->networks[s_index].end()
					&& it->second != NULL) {
				ExitNetwork* network = it->second;
				network->activate(history->state_vals_snapshot);
				outer_state_vals->at(s_index) += network->output->acti_vals[0];
			}
		}
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN
			&& run_helper.experiment_on_path) {
		int num_new_states = run_helper.experiment->num_new_states;

		history->new_state_vals_snapshot = run_helper.new_state_vals;

		for (int s_index = 0; s_index < num_new_states; s_index++) {
			map<StateDefinition*, ExitNetwork*>::iterator it = run_helper.experiment->exit_node_state_networks[s_index].find(outer_state_types[s_index]);
			if (it == run_helper.experiment->exit_node_state_networks[s_index].end()) {
				it = run_helper.experiment->exit_node_state_networks[s_index].insert(
					pair<StateDefinition*, ExitNetwork*>(outer_state_types[s_index],
						new ExitNetwork(this->exit_context))).first;
			}
			ExitNetwork* network = it->second;
			network->activate(history->state_vals_snapshot,
							  history->new_state_vals_snapshot);
			outer_state_vals->at(s_index) += network->output->acti_vals[0];
		}
	}
}

// TODO: train against seed evaluating to 0.0?
// - or don't seed, but simply train against existing (perhaps before applying changes from inner)?
