#include "fold.h"

using namespace std;

void Fold::experiment_pre_activate_helper(vector<double>& new_state_vals,
										  double& pre_predicted_score,
										  double& pre_scale_factor,
										  ScopeHistory* scope_history,
										  FoldHistory* history) {
	int scope_id = scope_history->scope->id;
	int scope_state_size = scope_history->scope->num_states;

	map<int, vector<vector<Network*>>>::iterator state_it = this->test_outer_state_networks.find(scope_id);
	map<int, vector<Network*>>::iterator score_it = this->test_outer_score_networks.find(scope_id);
	if (state_it == this->test_outer_state_networks.end()) {
		state_it = this->test_outer_state_networks.insert({scope_id, vector<vector<Network*>>()}).first;
		score_it = this->test_outer_score_networks.insert({scope_id, vector<Network*>()}).first;
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
					for (int s_index = 0; s_index < this->test_num_new_states; s_index++) {
						state_it->second[node_id].push_back(
							new Network(1,
										scope_state_size+this->test_num_new_states,
										20));
					}
					score_it->second[node_id] = new Network(0,
															scope_state_size+this->test_num_new_states,
															20);
				}

				vector<double> new_state_vals_snapshot(new_state_vals);

				for (int s_index = 0; s_index < this->test_num_new_states; s_index++) {
					Network* network = state_it->second[node_id][s_index];

					network->obs_input->acti_vals[0] = action_node_history->obs_snapshot;
					for (int ss_index = 0; ss_index < scope_state_size; ss_index++) {
						network->state_input->acti_vals[ss_index] = action_node_history->ending_state_snapshot[ss_index];
					}
					for (int ss_index = 0; ss_index < this->test_num_new_states; ss_index++) {
						network->state_input->acti_vals[scope_state_size+ss_index] = new_state_vals_snapshot[ss_index];
					}
					NetworkHistory* network_history = new NetworkHistory(network);
					network->activate(network_history);
					action_node_history->new_state_network_histories.push_back(network_history);

					new_state_vals[s_index] += network->output->acti_vals[0];
				}

				StateNetwork* score_network = score_it->second[node_id];

				for (int ss_index = 0; ss_index < scope_state_size; ss_index++) {
					score_network->state_input->acti_vals[ss_index] = action_node_history->ending_state_snapshot[ss_index];
				}
				for (int ss_index = 0; ss_index < this->test_num_new_states; ss_index++) {
					score_network->state_input->acti_vals[scope_state_size+ss_index] = new_state_vals_snapshot[ss_index];
				}
				NetworkHistory* score_network_history = new NetworkHistory(score_network);
				score_network->activate(score_network_history);
				action_node_history->new_score_network_history = score_network_history;
				action_node_history->new_score_network_update = score_network->output->acti_vals[0];

				pre_predicted_score += pre_scale_factor*action_node_history->score_network_update;
				pre_predicted_score += pre_scale_factor*score_network->output->acti_vals[0];
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				pre_scale_factor *= scope_node->scope_scale_mod->weight;

				experiment_pre_activate_helper(new_state_vals,
											   pre_predicted_score,
											   pre_scale_factor,
											   scope_node_history->inner_scope_history,
											   history);

				pre_scale_factor /= scope_node->scope_scale_mod->weight;
			}
		}
	}
}

void Fold::experiment_activate(Problem& problem,
							   vector<double>& state_vals,
							   double& predicted_score,
							   double& scale_factor,
							   RunHelper& run_helper,
							   FoldHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT_LEARN;
	run_helper.fold_scope_id = this->parent_scope_id;

	vector<double> new_state_vals(this->test_num_new_states, 0.0);

	double pre_predicted_score = solution->average_score;
	double pre_scale_factor = 1.0;
	experiment_pre_activate_helper(new_state_vals,
								   pre_predicted_score,
								   pre_scale_factor,
								   run_helper->full_history,
								   history);

	for (int s_index = 0; s_index < this->num_existing_states; s_index++) {
		this->test_starting_score_network->input->acti_vals[s_index] = state_vals[s_index];
	}
	for (int s_index = 0; s_index < this->test_num_new_states; s_index++) {
		this->test_starting_score_network->input->acti_vals[this->num_existing_states+s_index] = new_state_vals[s_index];
	}
	NetworkHistory* starting_score_network_history = new NetworkHistory(this->test_starting_score_network);
	this->test_starting_score_network->activate(starting_score_network_history);
	history->starting_score_network_history = starting_score_network_history;
	history->starting_score_update = this->test_starting_score_network->output->acti_vals[0];

	int num_inner_networks = 
}
