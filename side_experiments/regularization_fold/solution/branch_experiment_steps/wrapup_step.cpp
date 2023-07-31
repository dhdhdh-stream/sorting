#include "branch_experiment.h"

using namespace std;

void BranchExperiment::wrapup_pre_activate_helper(
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

					double temp_score_scale = (100000.0-this->state_iter)/100000.0;
					run_helper.predicted_score += temp_scale_factor*temp_score_scale*score_network->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				temp_scale_factor *= scope_node->scope_scale_mod->weight;

				wrapup_pre_activate_helper(temp_scale_factor,
										   run_helper,
										   scope_node_history->inner_scope_history);

				temp_scale_factor /= scope_node->scope_scale_mod->weight;
			}
		}
	}
}

void BranchExperiment::wrapup_activate(vector<double>& flat_vals,
									   vector<ForwardContextLayer>& context,
									   RunHelper& run_helper,
									   FoldHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_WRAPUP;

	// no longer need to save separate existing_predicted_score

	history->starting_state_vals_snapshot = context.back().state_vals;

	ScoreNetworkHistory* starting_score_network_history = new ScoreNetworkHistory(this->starting_score_network);
	this->starting_score_network->activate(history->starting_state_vals_snapshot,
										   starting_score_network_history);
	double branch_score = run_helper.scale_factor*this->starting_score_network->output->acti_vals[0];

	ScoreNetworkHistory* starting_original_score_network_history = new ScoreNetwork(this->starting_original_score_network);
	this->starting_original_score_network->activate(history->starting_state_vals_snapshot,
													starting_original_score_network_history);
	double original_score = run_helper.scale_factor*this->starting_original_score_network->output->acti_vals[0];

	ScoreNetworkHistory* starting_misguess_network_history = new ScoreNetworkHistory(this->starting_misguess_network);
	this->starting_misguess_network->activate(history->starting_state_vals_snapshot,
											  starting_misguess_network_history);

	ScoreNetworkHistory* starting_original_misguess_network_history = new ScoreNetworkHistory(this->starting_original_misguess_network);
	this->starting_original_misguess_network->activate(history->starting_state_vals_snapshot,
													   starting_original_misguess_network_history);

	double score_diff = branch_score - original_score;
	double score_val = score_diff / (solution->average_misguess*abs(run_helper.scale_factor));
	if (score_val > 0.1) {
		if (this->state_iter < 100000) {
			// ease-in in case of recursion
			double ease_in_scale = 0.5 + 0.5*(this->state_iter/100000.0);
			if (randuni() < ease_in_scale) {
				history->is_branch = true;
			} else {
				history->is_branch = false;
			}
		} else {
			history->is_branch = true;
		}
	} else if (score_val < -0.1) {
		history->is_branch = false;
	} else {
		double misguess_diff = this->starting_misguess_network->output->acti_vals[0]
			- this->starting_original_misguess_network->output->acti_vals[0];
		double misguess_val = misguess_diff / (solution->misguess_standard_deviation*abs(run_helper.scale_factor));
		if (misguess_val < -0.1) {
			if (this->state_iter < 100000) {
				// ease-in in case of recursion
				double ease_in_scale = 0.5 + 0.5*(this->state_iter/100000.0);
				if (randuni() < ease_in_scale) {
					history->is_branch = true;
				} else {
					history->is_branch = false;
				}
			} else {
				history->is_branch = true;
			}
		} else if (misguess_val > 0.1) {
			history->is_branch = false;
		} else {
			if (rand()%2 == 0) {
				if (this->state_iter < 100000) {
					// ease-in in case of recursion
					double ease_in_scale = 0.5 + 0.5*(this->state_iter/100000.0);
					if (randuni() < ease_in_scale) {
						history->is_branch = true;
					} else {
						history->is_branch = false;
					}
				} else {
					history->is_branch = true;
				}
			} else {
				history->is_branch = false;
			}
		}
	}

	if (!history->is_branch) {
		delete starting_score_network_history;
		delete starting_misguess_network_history;
		history->score_network_history = starting_original_score_network_history;
		history->misguess_network_history = starting_original_misguess_network_history;
		history->score_network_output = this->starting_original_score_network->output->acti_vals[0];
		history->misguess_network_output = this->starting_original_misguess_network->output->acti_vals[0];
	} else {
		delete starting_original_score_network_history;
		delete starting_original_misguess_network_history;
		history->score_network_history = starting_score_network_history;
		history->misguess_network_history = starting_misguess_network_history;
		history->score_network_output = this->starting_score_network->output->acti_vals[0];
		history->misguess_network_output = this->starting_misguess_network->output->acti_vals[0];

		if (this->state_iter < 100000) {
			double temp_scale_factor = 1.0;
			wrapup_pre_activate_helper(temp_scale_factor,
									   run_helper,
									   context[0].scope_history);
		}

		vector<double> new_state_vals(this->new_num_states, 0.0);
		for (int i_index = 0; i_index < (int)this->last_layer_indexes.size(); i_index++) {
			double val = context.back().state_vals[this->last_layer_indexes[i_index]];
			if (this->last_layer_has_transform[i_index]) {
				val = this->last_layer_transformations[i_index].forward(val);
			}
			new_state_vals[this->last_layer_target_indexes[i_index]] = val;
		}

		context.push_back(ForwardContextLayer());

		context.back().scope_id = -1;
		context.back().node_id = -1;

		context.back().state_vals = &new_state_vals;
		context.back().states_initialized = vector<bool>(this->new_num_states, true);
		// all states must be initialized

		history->step_obs_snapshots = vector<double>(this->num_steps, 0.0);
		history->step_starting_new_state_vals_snapshots = vector<vector<double>>(this->num_steps);
		history->step_ending_new_state_vals_snapshots = vector<vector<double>>(this->num_steps);
		history->step_score_network_histories = vector<ScoreNetworkHistory*>(this->num_steps, NULL);
		history->step_score_network_outputs = vector<double>(this->num_steps, 0.0);

		history->sequence_histories = vector<SequenceHistory*>(this->num_steps, NULL);

		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
				double obs = flat_vals.begin();

				history->step_obs_snapshots[a_index] = obs;
				history->step_starting_new_state_vals_snapshots[a_index] = new_state_vals;

				for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
					if (this->step_state_networks[a_index][s_index] == NULL) {
						// do nothing
					} else {
						StateNetwork* network = this->step_state_networks[a_index][s_index];
						network->activate(history->step_obs_snapshots[a_index],
										  history->step_starting_new_state_vals_snapshots[a_index]);
						new_state_vals[s_index] += network->output->acti_vals[0];
					}
				}

				history->step_ending_new_state_vals_snapshots[a_index] = new_state_vals;

				ScoreNetwork* score_network = this->step_score_networks[a_index];
				ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(score_network);
				score_network->activate(history->step_ending_new_state_vals_snapshots[a_index],
										score_network_history);
				history->step_score_network_histories[a_index] = score_network_history
				history->step_score_network_outputs[a_index] = score_network->output->acti_vals[0];

				predicted_score += run_helper.scale_factor*score_network->output->acti_vals[0];

				flat_vals.erase(flat_vals.begin());
			} else {
				run_helper.scale_factor *= this->sequence_scale_mods[a_index]->weight;

				// no longer need to track experiment_step_index

				SequenceHistory* sequence_history = new SequenceHistory(this->sequences[a_index]);
				history->sequence_histories[a_index] = sequence_history;
				this->sequences[a_index]->activate(flat_vals,
												   context,
												   history,
												   run_helper,
												   sequence_history);

				run_helper.scale_factor /= this->sequence_scale_mods[a_index]->weight;
			}
		}

		context.pop_back();
		for (int i_index = 0; i_index < (int)this->last_layer_indexes.size(); s_index++) {
			double val = new_state_vals[this->last_layer_target_indexes[i_index]];
			if (this->last_layer_has_transform[i_index]) {
				val = this->last_layer_transformations[i_index].backward(val);
			}
			context.back().state_vals[this->last_layer_indexes[i_index]] = val;
		}

		history->exit_state_vals_snapshot = vector<vector<double>>(this->exit_depth+1);
		for (int l_index = 0; l_index < this->exit_depth+1; l_index++) {
			history->exit_state_vals_snapshot[l_index] = context[
				context.size() - (this->exit_depth+1) + l_index].state_vals;
		}

		vector<double>* outer_state_vals = context[context.size() - (this->exit_depth+1)].state_vals;
		vector<bool>* outer_states_initialized = &(context[context.size() - (this->exit_depth+1)].states_initialized);

		for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
			if (outer_states_initialized->at(s_index)) {
				if (this->exit_networks[s_index] != NULL) {
					this->exit_networks[s_index]->activate(history->exit_state_vals_snapshot);
					outer_state_vals->at(s_index) += this->exit_networks[s_index]->output->acti_vals[0];
				}
			}
		}

		// no longer need to track experiment_context_index/experiment_on_path
	}
}

void BranchExperiment::wrapup_backprop(vector<BackwardContextLayer>& context,
									   double& scale_factor_error,
									   RunHelper& run_helper,
									   BranchExperimentHistory* history) {
	if (history->is_branch) {
		// no need to track context for WRAPUP

		for (int a_index = this->num_steps-1; a_index >= 0; a_index--) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
				double predicted_score_error = run_helper.target_val - run_helper.predicted_score;

				scale_factor_error += history->step_score_network_outputs[a_index]*predicted_score_error;
				/**
				 * - could already begin tracking new/separate scale_mod for new scope
				 *   - but won't bother for now (i.e., treating scale_mod as 1.0)
				 */

				this->step_score_networks[a_index]->backprop_weights_with_no_error_signal(
					run_helper.scale_factor*predicted_score_error,
					0.01,
					history->step_ending_new_state_vals_snapshots[a_index],
					history->step_score_network_histories[a_index]);

				run_helper.predicted_score -= run_helper.scale_factor*history->step_score_network_outputs[a_index];
			} else {
				run_helper.scale_factor *= this->sequence_scale_mods[a_index]->weight;

				double inner_scale_factor_error = 0.0;

				this->sequences[a_index]->backprop(context,
												   inner_scale_factor_error,
												   run_helper,
												   history->sequence_histories[a_index]);

				this->sequence_scale_mods[a_index]->backprop(inner_scale_factor_error, 0.002);

				scale_factor_error += this->sequence_scale_mods[a_index]->weight*inner_scale_factor_error;

				run_helper.scale_factor /= this->sequence_scale_mods[a_index]->weight;
			}
		}
	}

	double branch_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->score_network_output;
	double predicted_score_error = run_helper.target_val - branch_predicted_score;
	ScoreNetwork* score_network = history->score_network_history->network;
	score_network->backprop_weights_with_no_error_signal(
		run_helper.scale_factor*predicted_score_error,
		0.01,
		history->starting_state_vals_snapshot,
		history->score_network_history);

	double misguess_error = run_helper.final_misguess - history->misguess_network_output;
	ScoreNetwork* misguess_network = history->misguess_network_history->network;
	misguess_network->backprop_weights_with_no_error_signal(
		misguess_error,
		0.01,
		history->starting_state_vals_snapshot,
		history->misguess_network_history);

	if (history->is_branch) {
		this->branch_weight = 0.9999*this->branch_weight + 0.0001;
	} else {
		this->branch_weight = 0.9999*this->branch_weight + 0.0;
	}

	// no need to set run_helper.backprop_is_pre_experiment
}
