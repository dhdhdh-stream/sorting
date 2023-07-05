#include "branch_experiment.h"

using namespace std;

void BranchExperiment::cleanup_outer_activate_helper(
		double& temp_scale_factor,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<ScoreNetwork*>>::iterator score_it = this->action_node_score_networks.find(scope_id);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->scope_index;

				if (score_it != this->action_node_score_networks.end()
						&& node_id < score_it->second.size()
						&& score_it->second[node_id] != NULL) {
					ScoreNetwork* score_network = score_it->second[node_id];
					score_network->activate(action_node_history->ending_state_vals_snapshot);
					action_node_history->new_score_network_output = score_network->output->acti_vals[0];

					double temp_score_scale = (200000.0-this->experiment->state_iter)/200000.0;
					run_helper.predicted_score += temp_scale_factor*temp_score_scale*score_network->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				temp_scale_factor *= scope_node->scope_scale_mod->weight;

				if (i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					cleanup_outer_activate_helper(temp_scale_factor,
												  run_helper,
												  scope_node_history->inner_scope_history);

					temp_scale_factor /= scope_node->scope_scale_mod->weight;
				}
			}
		}
	}
}

void BranchExperiment::cleanup_activate(vector<double>& flat_vals,
										vector<ForwardContextLayer>& context,
										RunHelper& run_helper,
										FoldHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT;

	history->existing_predicted_score = predicted_score;

	double temp_scale_factor = 1.0;
	cleanup_outer_activate_helper(temp_scale_factor,
								  run_helper,
								  context[c_index].scope_history);

	history->starting_state_vals_snapshot = context.back().state_vals;

	// no need to activate starting_score_network until backprop

	// TODO: if score is not enough, change phase to UPDATE
	// - can even activate before cleanup_outer_activate_helper
	//   - actually, can update iter whether or not went down explore path

	run_helper.new_state_vals = vector<double>(this->final_num_new_states, 0.0);
	for (int s_index = 0; s_index < (int)this->last_layer_new_state_indexes.size(); s_index++) {
		run_helper.new_state_vals[s_index] = context.back().state_vals[this->last_layer_new_state_indexes[s_index]];
	}

	history->step_obs_snapshots = vector<double>(this->num_steps, 0.0);
	history->step_starting_new_state_vals_snapshots = vector<vector<double>>(this->num_steps);
	history->step_state_network_histories = vector<vector<StateNetworkHistory*>>(this->num_steps);
	history->step_ending_new_state_vals_snapshots = vector<vector<double>>(this->num_steps);
	history->step_score_network_histories = vector<ScoreNetworkHistory*>(this->num_steps, NULL);

	history->sequence_histories = vector<SequenceHistory*>(this->num_steps, NULL);

	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == EXPLORE_STEP_TYPE_ACTION) {
			double obs = flat_vals.begin();

			history->step_obs_snapshots[a_index] = obs;
			history->step_starting_new_state_vals_snapshots[a_index] = run_helper.new_state_vals;

			history->step_state_network_histories[a_index] = vector<StateNetworkHistory*>(this->final_num_new_states, NULL);
			for (int s_index = 0; s_index < this->final_num_new_states; s_index++) {
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

			predicted_score += run_helper.scale_factor*score_network->output->acti_vals[0];

			flat_vals.erase(flat_vals.begin());
		} else {
			run_helper.scale_factor *= this->sequence_scale_factors[a_index];

			run_helper.experiment_step_index = a_index;

			SequenceHistory* sequence_history = new SequenceHistory(this->sequences[a_index]);
			history->sequence_histories[a_index] = sequence_history;
			this->sequences[a_index]->experiment_activate(flat_vals,
														  context,
														  history,
														  run_helper,
														  sequence_history);

			run_helper.scale_factor /= this->sequence_scale_factors[a_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->last_layer_new_state_indexes.size(); s_index++) {
		context.back().state_vals[this->last_layer_new_state_indexes[s_index]] = run_helper.new_state_vals[s_index];
	}

	// including new experiment scope
	history->exit_state_vals_snapshot = vector<vector<double>>(this->exit_depth+1);
	for (int l_index = 0; l_index < this->exit_depth; l_index++) {
		history->exit_state_vals_snapshot[l_index] = context[
			context.size() - this->exit_depth + l_index].state_vals;
	}
	history->exit_state_vals_snapshot.back() = run_helper.new_state_vals;

	run_helper.new_state_vals.clear();

	vector<double>* outer_state_vals = context[context.size() - this->exit_depth].state_vals;
	vector<bool>* outer_states_initialized = &(context[context.size() - this->exit_depth].states_initialized);

	for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
		if (outer_states_initialized->at(s_index)) {
			if (this->exit_networks[s_index] != NULL) {
				this->exit_networks[s_index]->activate(history->exit_state_vals_snapshot);
				outer_state_vals->at(s_index) += this->exit_networks[s_index]->output->acti_vals[0];
			}
		}
	}
}
