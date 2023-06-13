#include "action_node.h"

using namespace std;



void ActionNode::activate(vector<double>& flat_vals,
						  vector<double>& state_vals,
						  vector<StateDefinition*>& state_types,
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	history->obs_snapshot = flat_vals.begin();
	history->starting_state_vals_snapshot = state_vals;

	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		if (state_types[s_index] != NULL) {
			map<StateDefinition*, Network*>::iterator it = this->state_networks[s_index].find(state_types[s_index]);
			if (it != this->state_networks[s_index].end()
					&& it->second != NULL) {
				Network* network = it->second;
				network->activate(history->obs_snapshot,
								  history->starting_state_vals_snapshot);
				state_vals[s_index] += network->output->acti_vals[0];
			}
		}
	}

	history->ending_state_vals_snapshot = state_vals;

	this->score_network->activate(history->ending_state_vals_snapshot);
	history->score_network_output = this->score_network->output->acti_vals[0];
	run_helper.predicted_score += run_helper.scale_factor*this->score_network->output->acti_vals[0];

	flat_vals.erase(flat_vals.begin());

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		int num_new_states = run_helper.experiment->num_new_states;

		if (run_helper.scope_state_networks->at(this->id).size() == 0) {
			for (int s_index = 0; s_index < num_new_states; s_index++) {
				run_helper.scope_state_networks->at(this->id).push_back(
					new Network(1,
								this->parent->num_states,
								num_new_states,
								20));
			}
			run_helper.scope_score_networks->at(this->id) = new Network(0,
																		this->parent->num_states,
																		num_new_states,
																		20);
		}

		history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

		for (int s_index = 0; s_index < num_new_states; s_index++) {
			if (run_helper.can_zero && rand()%5 == 0) {
				history->network_zeroed[s_index] = true;
			} else {
				Network* network = run_helper.scope_state_networks->at(this->id)[s_index];
				network->activate(history->obs_snapshot,
								  history->starting_state_vals_snapshot,
								  history->starting_new_state_vals_snapshot);
				run_helper.new_state_vals[s_index] += network->output->acti_vals[0];

				history->network_zeroed[s_index] = false;
			}
		}

		history->ending_new_state_vals_snapshot = run_helper.new_state_vals;

		Network* score_network = run_helper.scope_score_networks->at(this->id);
		score_network->activate(history->ending_state_vals_snapshot,
								history->ending_new_state_vals_snapshot);

		predicted_score += scale_factor*score_network->output->acti_vals[0];
	}
}
