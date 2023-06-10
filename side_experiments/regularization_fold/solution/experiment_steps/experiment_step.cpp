#include "fold.h"

using namespace std;

void Fold::experiment_global_activate_helper(bool on_path,
											 int remaining_scope_depth,
											 double& new_predicted_score,
											 double& pre_scale_factor,
											 RunHelper& run_helper,
											 ScopeHistory* scope_history,
											 FoldHistory* history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<Network*>>>::iterator state_it = this->action_node_state_networks[0].find(scope_id);
	map<int, vector<Network*>>::iterator score_it = this->action_node_score_networks[0].find(scope_id);
	if (state_it == this->action_node_state_networks[0].end()) {
		state_it = this->action_node_state_networks[0].insert({scope_id, vector<vector<Network*>>()}).first;
		score_it = this->action_node_score_networks[0].insert({scope_id, vector<Network*>()}).first;
	}

	int size_diff = (int)scope_history->scope->nodes.size() - (int)state_it->second.size();
	state_it->second.insert(state_it->second.end(), size_diff, vector<Network*>());
	score_it->second.insert(score_it->second.end(), size_diff, NULL);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

				if (state_it->second[node_id].size() == 0) {
					for (int s_index = 0; s_index < this->num_inner_inputs+5; s_index++) {
						state_it->second[node_id].push_back(
							new Network(1,
										scope_history->scope->num_states,
										this->num_inner_inputs+5,
										20));
					}
					score_it->second[node_id] = new Network(0,
															scope_history->scope->num_states,
															this->num_inner_inputs+5,
															20);
				}

				action_node_history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

				action_node_history->network_zeroed = vector<bool>(this->num_inner_inputs+5);
				for (int s_index = 0; s_index < this->num_inner_inputs+5; s_index++) {
					if (history->can_zero && rand()%5 == 0) {
						action_node_history->network_zeroed[s_index] = true;
					} else {
						Network* network = state_it->second[node_id][s_index];
						network->activate(action_node_history->obs_snapshot,
										  action_node_history->starting_state_vals_snapshot,
										  action_node_history->starting_new_state_vals_snapshot);
						run_helper.new_state_vals[s_index] += network->output->acti_vals[0];

						action_node_history->network_zeroed[s_index] = false;
					}
				}

				action_node_history->ending_new_state_vals_snapshot = run_helper.new_state_vals;

				Network* score_network = score_it->second[node_id];
				score_network->activate(action_node_history->ending_state_vals_snapshot,
										action_node_history->ending_new_state_vals_snapshot);

				new_predicted_score += pre_scale_factor*action_node_history->score_network_output;
				new_predicted_score += pre_scale_factor*score_network->output->acti_vals[0];
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				pre_scale_factor *= scope_node->scope_scale_mod->weight;

				if (on_path
						&& i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					if (remaining_scope_depth == 1) {
						// do nothing
					} else {
						experiment_global_activate_helper(true,
														  remaining_scope_depth-1,
														  new_predicted_score,
														  pre_scale_factor,
														  run_helper,
														  scope_node_history->inner_scope_history,
														  history);
					}
				} else {
					experiment_global_activate_helper(false,
													  remaining_scope_depth,
													  new_predicted_score,
													  pre_scale_factor,
													  run_helper,
													  scope_node_history->inner_scope_history,
													  history);
				}

				pre_scale_factor /= scope_node->scope_scale_mod->weight;
			}
		}
	}
}

void Fold::experiment_context_activate_helper(int context_index,
											  double& new_predicted_score,
											  double& pre_scale_factor,
											  RunHelper& run_helper,
											  ScopeHistory* scope_history
											  FoldHistory* history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<Network*>>>::iterator state_it = this->action_node_state_networks[1+context_index].find(scope_id);
	map<int, vector<Network*>>::iterator score_it = this->action_node_score_networks[1+context_index].find(scope_id);
	if (state_it == this->action_node_state_networks[1+context_index].end()) {
		state_it = this->action_node_state_networks[1+context_index].insert({scope_id, vector<vector<Network*>>()}).first;
		score_it = this->action_node_score_networks[1+context_index].insert({scope_id, vector<Network*>()}).first;
	}

	int size_diff = (int)scope_history->scope->nodes.size() - (int)state_it->second.size();
	state_it->second.insert(state_it->second.end(), size_diff, vector<Network*>());
	score_it->second.insert(score_it->second.end(), size_diff, NULL);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

				if (state_it->second[node_id].size() == 0) {
					for (int s_index = 0; s_index < this->num_inner_inputs+5; s_index++) {
						state_it->second[node_id].push_back(
							new Network(1,
										scope_history->scope->num_states,
										this->num_inner_inputs+5,
										20));
					}
					score_it->second[node_id] = new Network(0,
															scope_history->scope->num_states,
															this->num_inner_inputs+5,
															20);
				}

				action_node_history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

				action_node_history->network_zeroed = vector<bool>(this->num_inner_inputs+5);
				for (int s_index = 0; s_index < this->num_inner_inputs+5; s_index++) {
					if (history->can_zero && rand()%5 == 0) {
						action_node_history->network_zeroed[s_index] = true;
					} else {
						Network* network = state_it->second[node_id][s_index];
						network->activate(action_node_history->obs_snapshot,
										  action_node_history->starting_state_vals_snapshot,
										  action_node_history->starting_new_state_vals_snapshot);
						run_helper.new_state_vals[s_index] += network->output->acti_vals[0];

						action_node_history->network_zeroed[s_index] = false;
					}
				}

				action_node_history->ending_new_state_vals_snapshot = run_helper.new_state_vals;

				Network* score_network = score_it->second[node_id];
				score_network->activate(action_node_history->ending_state_vals_snapshot,
										action_node_history->ending_new_state_vals_snapshot);

				new_predicted_score += pre_scale_factor*action_node_history->score_network_output;
				new_predicted_score += pre_scale_factor*score_network->output->acti_vals[0];
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				pre_scale_factor *= scope_node->scope_scale_mod->weight;

				if (i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					experiment_context_activate_helper(context_index,
													   new_predicted_score,
													   pre_scale_factor,
													   run_helper,
													   scope_node_history->inner_scope_history,
													   history);
				}

				pre_scale_factor /= scope_node->scope_scale_mod->weight;
			}
		}
	}
}

void Fold::experiment_activate(vector<double>& flat_vals,
							   vector<double>& state_vals,	// for starting_score_network
							   vector<TypeDefinition*>& state_types,
							   double& predicted_score,
							   double& scale_factor,
							   vector<ContextLayer>& context,
							   RunHelper& run_helper,
							   FoldHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT_LEARN;
	run_helper.experiment_scope_id = this->scope_context.back();

	if (rand()%5 == 0) {
		history->can_zero = true;
	} else {
		history->can_zero = false;
	}

	run_helper.new_state_vals = vector<double>(this->num_inner_inputs+5, 0.0);

	double new_predicted_score = this->replace_average_score;
	double pre_scale_factor = 1.0;

	if (curr_scope_context.size() > this->scope_context.size()) {
		experiment_global_activate_helper(true,
										  curr_scope_context.size() - this->scope_context.size(),
										  new_predicted_score,
										  pre_scale_factor,
										  run_helper,
										  context[0]->scope_history,
										  history);
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		experiment_context_activate_helper(c_index,
										   new_predicted_score,
										   pre_scale_factor,
										   run_helper,
										   context[curr_scope_context.size() - this->scope_context.size() + c_index]->scope_history,
										   history);

		for (int i_index = 0; i_index < this->num_inner_inputs; i_index++) {
			if (this->init_inner_input_scope_depths[i_index] == c_index) {
				vector<double>* scope_state_vals = context[
					context_state_vals.size() - this->scope_context.size() + this->init_inner_input_scope_depths[i_index]]->state_vals;
				new_state_vals[i_index] += scope_state_vals->at(this->init_inner_input_input_indexes[i_index]);
				// can be uninitialized (i.e., 0.0)
			}
		}
	}

	history->starting_state_vals_snapshot = state_vals;
	history->starting_new_state_vals_snapshot = run_helper.new_state_vals;
	history->existing_predicted_score = predicted_score;
	predicted_score = new_predicted_score;

	// no need to activate starting_score_network until backprop

	history->sequence_obs_snapshots = vector<double>(this->sequence_length, 0.0);
	history->sequence_starting_new_state_vals_snapshots = vector<vector<double>>(this->sequence_length);
	history->sequence_network_zeroed = vector<vector<bool>>(this->sequence_length);
	history->sequence_ending_new_state_vals_snapshots = vector<vector<double>>(this->sequence_length);

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (this->step_types[a_index] == EXPLORE_STEP_TYPE_ACTION) {
			double obs = flat_vals.begin();

			history->sequence_obs_snapshots[a_index] = obs;
			history->sequence_starting_new_state_vals_snapshots[a_index] = run_helper.new_state_vals;

			history->sequence_network_zeroed[a_index] = vector<bool>(this->num_inner_inputs+5);
			for (int s_index = 0; s_index < this->num_inner_inputs+5; s_index++) {
				if (history->can_zero && rand()%5 == 0) {
					history->sequence_network_zeroed[a_index][s_index] = true;
				} else {
					Network* network = this->sequence_state_networks[a_index][s_index];
					network->activate(history->sequence_obs_snapshots[a_index],
									  history->sequence_starting_new_state_vals_snapshots[a_index]);
					run_helper.new_state_vals[s_index] += network->output->acti_vals[0];

					history->sequence_network_zeroed[a_index][s_index] = false;
				}
			}

			history->sequence_ending_new_state_vals_snapshots[a_index] = run_helper.new_state_vals;

			Network* score_network = this->sequence_score_networks[a_index];
			score_network->activate(history->sequence_ending_new_state_vals_snapshots[a_index]);
			predicted_score += scale_factor*score_network->output->acti_vals[0];

			flat_vals.erase(flat_vals.begin());
		} else if (this->step_types[a_index] == EXPLORE_STEP_TYPE_EXISTING_SCOPE) {
			scale_factor *= this->inner_scope_scale_mods[f_index]->weight;

			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];

			vector<double> input_vals(inner_scope->num_states, 0.0);
			vector<TypeDefinition*> input_types(inner_scope->num_states, NULL);
			for (int i_index = 0; i_index < (int)this->inner_input_indexes[a_index].size(); i_index++) {
				if (this->inner_input_indexes[a_index][i_index] != -1) {
					double val = run_helper.new_state_vals[this->inner_input_indexes[a_index][i_index]];
					if (this->inner_input_transformation[a_index][i_index] != NULL) {
						val = this->inner_input_transformation[a_index][i_index]->forward(val);
					}
					input_vals[i_index] = val;
					run_helper.new_state_vals[this->inner_input_indexes[a_index][i_index]] = 0.0;
					/**
					 * - set new_state_val to 0.0 if used
					 *   - OK, since units should remain the same due to use after inner
					 */

					input_types[i_index] = this->inner_input_types[a_index][i_index];
				}
			}

			vector<ContextLayer> inner_context;
			int early_exit_depth;
			int early_exit_node_id;

			ScopeHistory* scope_history = new ScopeHistory(inner_scope);
			history->inner_scope_histories[a_index] = scope_history;
			inner_scope->activate(flat_vals,
								  input_vals,
								  input_types,
								  predicted_score,
								  scale_factor,
								  inner_context,
								  early_exit_depth,
								  early_exit_node_id,
								  run_helper,
								  scope_history);

			for (int i_index = 0; i_index < (int)this->inner_input_indexes[a_index].size(); i_index++) {
				if (this->inner_input_indexes[a_index][i_index] != -1) {
					run_helper.new_state_vals[this->inner_input_indexes[a_index][i_index]] += input_vals[i_index];
				}
			}

			scale_factor /= this->inner_scope_scale_mods[f_index]->weight;
		} else {
			// this->step_types[a_index] == EXPLORE_STEP_TYPE_FETCH

			// TODO: fetch output from context
		}
	}
}
