#include "branch_experiment.h"

using namespace std;

void BranchExperiment::experiment_outer_activate_helper(
		int context_index,
		double& temp_scale_factor,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator state_it = this->action_node_state_networks.find(scope_id);
	map<int, vector<ScoreNetwork*>>::iterator score_it = this->action_node_score_networks.find(scope_id);

	if (this->state == EXPERIMENT_STATE_EXPERIMENT) {
		if (state_it == this->action_node_state_networks.end()) {
			state_it = this->action_node_state_networks.insert({scope_id, vector<vector<StateNetwork*>>()}).first;
			score_it = this->action_node_score_networks.insert({scope_id, vector<ScoreNetwork*>()}).first;
		}

		int size_diff = (int)scope_history->scope->nodes.size() - (int)state_it->second.size();
		state_it->second.insert(state_it->second.end(), size_diff, vector<StateNetwork*>());
		score_it->second.insert(score_it->second.end(), size_diff, NULL);

		map<int, int>::iterator seen_it = this->scope_furthest_layer_seen_in.find(scope_id);
		if (seen_it == this->scope_furthest_layer_seen_in.end()) {
			seen_it[scope_id] = context_index;
		} else {
			if (seen_it->second > context_index) {
				seen_it->second = context_index;
			}
		}
	}

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->scope_index;

				if (this->state == EXPERIMENT_STATE_EXPERIMENT) {
					if (state_it->second[node_id].size() == 0) {
						for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
							state_it->second[node_id].push_back(
								new StateNetwork(scope_history->scope->num_states,
												 NUM_NEW_STATES,
												 20));
						}
						score_it->second[node_id] = new ScoreNetwork(scope_history->scope->num_states,
																	 NUM_NEW_STATES,
																	 20);
					}
				}

				if (state_it != this->action_node_state_networks.end()
						&& node_id < state_it->second.size()
						&& state_it->second[node_id].size() != 0) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

					action_node_history->experiment_context_index = context_index;
					action_node_history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

					action_node_history->new_state_network_histories = vector<StateNetworkHistory*>(NUM_NEW_STATES, NULL);
					for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
						if (run_helper.can_zero && rand()%5 == 0) {
							// do nothing
						} else if (state_it->second[node_id][s_index] == NULL) {
							// do nothing
						} else {
							StateNetwork* network = state_it->second[node_id][s_index];
							StateNetworkHistory* network_history = new StateNetworkHistory(network);
							network->activate(action_node_history->obs_snapshot,
											  action_node_history->starting_state_vals_snapshot,
											  action_node_history->starting_new_state_vals_snapshot,
											  network_history);
							action_node_history->new_state_network_histories[s_index] = network_history;
							run_helper.new_state_vals[s_index] += network->output->acti_vals[0];
						}
					}

					action_node_history->ending_new_state_vals_snapshot = run_helper.new_state_vals;

					ScoreNetwork* score_network = score_it->second[node_id];
					ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(score_network);
					score_network->activate(action_node_history->ending_state_vals_snapshot,
											action_node_history->ending_new_state_vals_snapshot,
											score_network_history);
					action_node_history->new_score_network_history = score_network_history;
					action_node_history->new_score_network_output = score_network->output->acti_vals[0];

					run_helper.predicted_score += temp_scale_factor*score_network->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				temp_scale_factor *= scope_node->scope_scale_mod->weight;

				if (i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					experiment_outer_activate_helper(context_index,
													 temp_scale_factor,
													 run_helper,
													 scope_node_history->inner_scope_history);

					temp_scale_factor /= scope_node->scope_scale_mod->weight;
				}
			}
		}
	}
}

void BranchExperiment::experiment_activate(vector<double>& flat_vals,
										   vector<ForwardContextLayer>& context,
										   RunHelper& run_helper,
										   FoldHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT;

	run_helper.experiment_scope_id = this->scope_context.back();
	run_helper.is_recursive = false;

	history->existing_predicted_score = predicted_score;

	run_helper.experiment = this;
	if (rand()%5 == 0) {
		run_helper.can_zero = true;
	} else {
		run_helper.can_zero = false;
	}
	run_helper.new_state_vals = vector<double>(NUM_NEW_STATES, 0.0);

	double temp_scale_factor = 1.0;

	int context_size_diff = (int)context.size() - (int)this->scope_context.size();
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		experiment_outer_activate_helper(c_index - context_size_diff,
										 temp_scale_factor,
										 run_helper,
										 context[c_index].scope_history);
	}

	history->starting_state_vals_snapshot = context.back().state_vals;
	history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

	// no need to activate starting_score_network until backprop

	history->step_obs_snapshots = vector<double>(this->num_steps, 0.0);
	history->step_starting_new_state_vals_snapshots = vector<vector<double>>(this->num_steps);
	history->step_state_network_histories = vector<vector<StateNetworkHistory*>>(this->num_steps);
	history->step_ending_new_state_vals_snapshots = vector<vector<double>>(this->num_steps);
	history->step_score_network_histories = vector<ScoreNetworkHistory*>(this->num_steps, NULL);

	history->sequence_histories = vector<SequenceHistory*>(this->num_steps, NULL);

	run_helper.experiment_context_index = this->scope_context.size()+1;
	run_helper.experiment_on_path = false;

	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == EXPLORE_STEP_TYPE_ACTION) {
			double obs = flat_vals.begin();

			history->step_obs_snapshots[a_index] = obs;
			history->step_starting_new_state_vals_snapshots[a_index] = run_helper.new_state_vals;

			history->step_state_network_histories[a_index] = vector<StateNetworkHistory*>(NUM_NEW_STATES, NULL);
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				if (history->can_zero && rand()%5 == 0) {
					// do nothing
				} else if (this->step_state_networks[a_index][s_index] == NULL) {
					// do nothing
				} else {
					StateNetwork* network = this->step_state_networks[a_index][s_index];
					StateNetworkHistory* network_history = new StateNetworkHistory(network);
					network->activate(history->step_obs_snapshots[a_index],
									  history->step_starting_new_state_vals_snapshots[a_index],
									  network_history);
					history->step_state_network_histories[a_index][s_index] = network_history;
					run_helper.new_state_vals[s_index] += network->output->acti_vals[0];
				}
			}

			history->step_ending_new_state_vals_snapshots[a_index] = run_helper.new_state_vals;

			ScoreNetwork* score_network = this->step_score_networks[a_index];
			ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(score_network);
			score_network->activate(history->step_ending_new_state_vals_snapshots[a_index],
									score_network_history);
			history->step_score_network_histories[a_index] = score_network_history
			history->step_score_network_outputs[a_index] = score_network->output->acti_vals[0];

			predicted_score += scale_factor*score_network->output->acti_vals[0];

			flat_vals.erase(flat_vals.begin());
		} else {
			run_helper.experiment_step_index = a_index;

			SequenceHistory* sequence_history = new SequenceHistory(this->sequences[a_index]);
			history->sequence_histories[a_index] = sequence_history;
			this->sequences[a_index]->experiment_activate(flat_vals,
														  context,
														  history,
														  run_helper,
														  sequence_history);
		}
	}

	history->exit_state_vals_snapshot = vector<vector<double>>(this->exit_depth+1);
	for (int l_index = 0; l_index < this->exit_depth+1; l_index++) {
		history->exit_state_vals_snapshot[l_index] = context[
			context.size() - (this->exit_depth+1) + l_index].state_vals;
	}

	vector<double>* outer_state_vals = context[context.size() - (this->exit_depth+1)].state_vals;
	vector<bool>* outer_states_initialized = &(context[context.size() - (this->exit_depth+1)].states_initialized);

	history->exit_network_histories = vector<ExitNetworkHistory*>(this->exit_networks.size(), NULL);
	for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
		if (outer_states_initialized->at(s_index)) {
			if (this->exit_networks[s_index] != NULL) {
				ExitNetworkHistory* network_history = new ExitNetworkHistory(this->exit_networks[s_index]);
				this->exit_networks[s_index]->activate(history->exit_state_vals_snapshot,
													   network_history);
				history->network_histories[s_index] = network_history;
				outer_state_vals->at(s_index) += this->exit_networks[s_index]->output->acti_vals[0];
			}
		}
	}

	run_helper.experiment_context_index--;
	run_helper.experiment_step_index = -1;
	run_helper.experiment_on_path = true;
}
