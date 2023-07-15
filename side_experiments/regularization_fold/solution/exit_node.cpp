#include "exit_node.h"

using namespace std;



void ExitNode::activate(vector<ForwardContextLayer>& context,
						RunHelper& run_helper,
						ExitNodeHistory* history) {
	history->state_vals_snapshot = vector<vector<double>>(this->exit_depth+1);
	for (int l_index = 0; l_index < this->exit_depth+1; l_index++) {
		history->state_vals_snapshot[l_index] = context[
			context.size() - (this->exit_depth+1) + l_index].state_vals;
	}

	vector<double>* outer_state_vals = context[context.size() - (this->exit_depth+1)].state_vals;
	vector<bool>* outer_states_initialized = &(context[context.size() - (this->exit_depth+1)].states_initialized);

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		history->network_histories = vector<ExitNetworkHistory*>(this->networks.size(), NULL);
		for (int s_index = 0; s_index < (int)this->networks.size(); s_index++) {
			if (outer_states_initialized->at(s_index)) {
				ExitNetwork* network = this->networks[s_index];
				ExitNetworkHistory* network_history = new ExitNetworkHistory(network);
				network->activate(history->state_vals_snapshot,
								  network_history);
				history->network_histories[s_index] = network_history;
				outer_state_vals->at(this->target_indexes[s_index]) += network->output->acti_vals[0];
			}
		}
	} else {
		for (int s_index = 0; s_index < (int)this->networks.size(); s_index++) {
			if (outer_states_initialized->at(s_index)) {
				ExitNetwork* network = this->networks[s_index];
				network->activate(history->state_vals_snapshot);
				outer_state_vals->at(this->target_indexes[s_index]) += network->output->acti_vals[0];
			}
		}
	}
}

void ExitNode::backprop(vector<BackwardContextLayer>& context,
						RunHelper& run_helper,
						ExitNodeHistory* history) {
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		if (!run_helper.backprop_is_pre_experiment) {
			vector<vector<double>*> state_errors(this->exit_depth+1);
			for (int l_index = 0; l_index < this->exit_depth+1; l_index++) {
				state_errors[l_index] = context[
					context.size() - (this->exit_depth+1) + l_index].state_errors;
			}

			vector<double> outer_state_errors_snapshot = *(context[context.size() - (this->exit_depth+1)].state_errors);

			for (int s_index = 0; s_index < (int)this->networks.size(); s_index++) {
				if (history->network_histories[s_index] != NULL) {
					ExitNetwork* network = history->network_histories[s_index]->network;
					network->backprop_errors_with_no_weight_change(
						outer_state_errors_snapshot[this->target_indexes[s_index]],
						state_errors,
						history->state_vals_snapshot,
						history->network_histories[s_index]);
				}
			}
		}
	}
}


