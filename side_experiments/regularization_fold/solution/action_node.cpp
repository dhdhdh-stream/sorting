#include "action_node.h"

using namespace std;



void ActionNode::activate(vector<double>& flat_vals,
						  vector<ForwardContextLayer>& context,
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	history->obs_snapshot = flat_vals.begin();
	history->starting_state_vals_snapshot = *(context.back().state_vals);

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_CLEAN) {
		history->state_network_histories = vector<StateNetworkHistory*>(this->state_networks.size(), NULL);
		for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
			if (context.back().states_initialized[s_index]) {
				if (this->state_networks[s_index] != NULL) {
					StateNetworkHistory* network_history = new StateNetworkHistory(this->state_networks[s_index]);
					this->state_networks[s_index]->activate(history->obs_snapshot,
															history->starting_state_vals_snapshot,
															network_history);
					history->state_network_histories[s_index] = network_history;
					context.back().state_vals->at(s_index) += this->state_networks[s_index]->output->acti_vals[0];
				}
			}
		}
	} else {
		for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
			if (context.back().states_initialized[s_index]) {
				if (this->state_networks[s_index] != NULL) {
					this->state_networks[s_index]->activate(history->obs_snapshot,
															history->starting_state_vals_snapshot);
					context.back().state_vals->at(s_index) += this->state_networks[s_index]->output->acti_vals[0];
				}
			}
		}
	}

	if (run_helper.explore_phase != EXPLORE_PHASE_EXPLORE) {
		history->ending_state_vals_snapshot = *(context.back().state_vals);

		ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->score_network);
		this->score_network->activate(history->ending_state_vals_snapshot,
									  score_network_history);
		history->score_network_history = score_network_history;
		history->score_network_output = this->score_network->output->acti_vals[0];
		run_helper.predicted_score += run_helper.scale_factor*this->score_network->output->acti_vals[0];
	}
	// don't actually need to save history during EXPLORE_PHASE_EXPERIMENT_MEASURE, but won't special case for now

	flat_vals.erase(flat_vals.begin());

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		if (run_helper.scope_state_networks->at(this->id).size() == 0) {
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				run_helper.scope_state_networks->at(this->id).push_back(
					new StateNetwork(this->parent->num_states,
									 NUM_NEW_STATES,
									 20));
			}
			run_helper.scope_score_networks->at(this->id) = new ScoreNetwork(this->parent->num_states,
																			 NUM_NEW_STATES,
																			 20);
		}

		history->experiment_context_index = run_helper.experiment_context_index;
		history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

		history->new_state_network_histories = vector<StateNetworkHistory*>(NUM_NEW_STATES, NULL);
		for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
			if (run_helper.can_zero && rand()%5 == 0) {
				// do nothing
			} else {
				StateNetwork* network = run_helper.scope_state_networks->at(this->id)[s_index];
				StateNetworkHistory* network_history = new StateNetworkHistory(network);
				network->activate(history->obs_snapshot,
								  history->starting_state_vals_snapshot,
								  history->starting_new_state_vals_snapshot,
								  network_history);
				history->new_state_network_histories[s_index] = network_history;
				run_helper.new_state_vals[s_index] += network->output->acti_vals[0];
			}
		}

		history->ending_new_state_vals_snapshot = run_helper.new_state_vals;

		ScoreNetwork* score_network = run_helper.scope_score_networks->at(this->id);
		ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(score_network);
		score_network->activate(history->ending_state_vals_snapshot,
								history->ending_new_state_vals_snapshot,
								score_network_history);
		history->new_score_network_history = score_network_history;
		history->new_score_network_output = score_network->output->acti_vals[0];

		run_helper.predicted_score += scale_factor*score_network->output->acti_vals[0];
	} else if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_CLEAN) {
		if (this->id < run_helper.scope_state_networks->size()
				&& run_helper.scope_state_networks->at(this->id).size() > 0) {
			history->experiment_context_index = run_helper.experiment_context_index;
			history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

			history->new_state_network_histories = vector<StateNetworkHistory*>(NUM_NEW_STATES, NULL);
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				if (run_helper.can_zero && rand()%5 == 0) {
					// do nothing
				} else {
					StateNetwork* network = run_helper.scope_state_networks->at(this->id)[s_index];
					if (network != NULL) {
						StateNetworkHistory* network_history = new StateNetworkHistory(network);
						network->activate(history->obs_snapshot,
										  history->starting_state_vals_snapshot,
										  history->starting_new_state_vals_snapshot,
										  network_history);
						history->new_state_network_histories[s_index] = network_history;
						run_helper.new_state_vals[s_index] += network->output->acti_vals[0];
					}
				}
			}

			history->ending_new_state_vals_snapshot = run_helper.new_state_vals;

			ScoreNetwork* score_network = run_helper.scope_score_networks->at(this->id);
			ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(score_network);
			score_network->activate(history->ending_state_vals_snapshot,
									history->ending_new_state_vals_snapshot,
									score_network_history);
			history->new_score_network_history = score_network_history;
			history->new_score_network_output = score_network->output->acti_vals[0];

			run_helper.predicted_score += scale_factor*score_network->output->acti_vals[0];
		}
	}
}
