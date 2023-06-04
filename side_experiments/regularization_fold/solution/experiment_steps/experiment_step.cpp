#include "fold.h"

using namespace std;

void Fold::experiment_global_activate_helper(bool on_path,
											 int remaining_scope_depth,
											 vector<double>& new_state_vals,
											 double& new_predicted_score,
											 double& pre_scale_factor,
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
					for (int s_index = 0; s_index < this->num_new_states; s_index++) {
						state_it->second[node_id].push_back(
							new Network(1,
										scope_history->scope->num_states,
										this->num_new_states,
										20));
					}
					score_it->second[node_id] = new Network(0,
															scope_history->scope->num_states,
															this->num_new_states,
															20);
				}

				action_node_history->starting_new_state_vals_snapshot = new_state_vals;

				action_node_history->network_zeroed = vector<bool>(this->num_new_states);
				for (int s_index = 0; s_index < this->num_new_states; s_index++) {
					if (history->can_zero && rand()%5 == 0) {
						action_node_history->network_zeroed[s_index] = true;
					} else {
						Network* network = state_it->second[node_id][s_index];
						network->activate(action_node_history->obs_snapshot,
										  action_node_history->starting_state_vals_snapshot,
										  action_node_history->starting_new_state_vals_snapshot);
						new_state_vals[s_index] += network->output->acti_vals[0];

						action_node_history->network_zeroed[s_index] = false;
					}
				}

				action_node_history->ending_new_state_vals_snapshot = new_state_vals;

				Network* score_network = score_it->second[node_id];
				score_network->activate(action_node_history->ending_state_vals_snapshot,
										action_node_history->ending_new_state_vals_snapshot);

				new_predicted_score += pre_scale_factor*action_node_history->score_network_output;
				new_predicted_score += pre_scale_factor*action_node_history->experiment_score_network_output;
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
														  new_state_vals,
														  new_predicted_score,
														  pre_scale_factor,
														  scope_node_history->inner_scope_history,
														  history);
					}
				} else {
					experiment_global_activate_helper(false,
													  remaining_scope_depth,
													  new_state_vals,
													  new_predicted_score,
													  pre_scale_factor,
													  scope_node_history->inner_scope_history,
													  history);
				}

				pre_scale_factor /= scope_node->scope_scale_mod->weight;
			}
		}
	}
}

void Fold::experiment_context_activate_helper(int context_index,
											  vector<double>& new_state_vals,
											  double& new_predicted_score,
											  double& pre_scale_factor,
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
					for (int s_index = 0; s_index < this->num_new_states; s_index++) {
						state_it->second[node_id].push_back(
							new Network(1,
										scope_history->scope->num_states,
										this->num_new_states,
										20));
					}
					score_it->second[node_id] = new Network(0,
															scope_history->scope->num_states,
															this->num_new_states,
															20);
				}

				action_node_history->starting_new_state_vals_snapshot = new_state_vals;

				action_node_history->network_zeroed = vector<bool>(this->num_new_states);
				for (int s_index = 0; s_index < this->num_new_states; s_index++) {
					if (history->can_zero && rand()%5 == 0) {
						action_node_history->network_zeroed[s_index] = true;
					} else {
						Network* network = state_it->second[node_id][s_index];
						network->activate(action_node_history->obs_snapshot,
										  action_node_history->starting_state_vals_snapshot,
										  action_node_history->starting_new_state_vals_snapshot);
						new_state_vals[s_index] += network->output->acti_vals[0];

						action_node_history->network_zeroed[s_index] = false;
					}
				}

				action_node_history->ending_new_state_vals_snapshot = new_state_vals;

				Network* score_network = score_it->second[node_id];
				score_network->activate(action_node_history->ending_state_vals_snapshot,
										action_node_history->ending_new_state_vals_snapshot);

				new_predicted_score += pre_scale_factor*action_node_history->score_network_output;
				new_predicted_score += pre_scale_factor*action_node_history->experiment_score_network_output;
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				pre_scale_factor *= scope_node->scope_scale_mod->weight;

				if (i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					experiment_context_activate_helper(context_index,
													   new_state_vals,
													   new_predicted_score,
													   pre_scale_factor,
													   scope_node_history->inner_scope_history,
													   history);
				}

				pre_scale_factor /= scope_node->scope_scale_mod->weight;
			}
		}
	}
}

void Fold::experiment_activate(vector<double>& flat_vals,
							   vector<double>& state_vals,
							   double& predicted_score,
							   double& scale_factor,
							   vector<int>& curr_scope_context,
							   vector<int>& curr_node_context,
							   vector<ScopeHistory*>& context_histories,
							   vector<vector<double>*>& context_state_vals,
							   RunHelper& run_helper,
							   FoldHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT_LEARN;
	run_helper.fold_scope_id = this->parent_scope_id;

	if (rand()%5 == 0) {
		history->can_zero = true;
	} else {
		history->can_zero = false;
	}

	vector<double> new_state_vals(this->num_new_states, 0.0);
	double new_predicted_score = solution->average_score;

	double pre_scale_factor = 1.0;

	if (curr_scope_context.size() > this->scope_context.size()) {
		experiment_global_activate_helper(true,
										  curr_scope_context.size() - this->scope_context.size(),
										  new_state_vals,
										  new_predicted_score,
										  pre_scale_factor,
										  context_histories[0],
										  history);
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		experiment_context_activate_helper(c_index,
										   new_state_vals,
										   new_predicted_score,
										   pre_scale_factor,
										   context_histories[curr_scope_context.size() - this->scope_context.size() + c_index],
										   history);
	}

	history->starting_state_vals_snapshot = state_vals;
	history->starting_new_state_vals_snapshot = new_state_vals;

	// no need to activate starting_score_network until backprop

	vector<double> local_inner_input_vals(this->reverse_local_inner_input_mapping.size());
	for (int l_index = 0; l_index < (int)this->reverse_local_inner_input_mapping.size(); l_index++) {
		local_inner_input_vals[l_index] = context_state_vals[context_state_vals.size()-this->scope_context.size()
			+this->inner_input_scope_depths[this->reverse_local_inner_input_mapping[l_index]]][
			this->inner_input_input_indexes[this->reverse_local_inner_input_mapping[l_index]]];
	}

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			// don't update state networks pre scope node
			// - intuitively, state is tied to the world
			//   - not decisions (i.e., branching)
		} else {

		}
	}

	for (int l_index = 0; l_index < (int)this->reverse_local_inner_input_mapping.size(); l_index++) {
		context_state_vals[context_state_vals.size()-this->scope_context.size()
			+this->inner_input_scope_depths[this->reverse_local_inner_input_mapping[l_index]]][
			this->inner_input_input_indexes[this->reverse_local_inner_input_mapping[l_index]]] = local_inner_input_vals[l_index];
	}


}
