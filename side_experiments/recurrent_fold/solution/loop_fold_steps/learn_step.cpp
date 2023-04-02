#include "loop_fold.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void LoopFold::learn_outer_scope_activate_helper(vector<double>& new_outer_state_vals,
												 ScopeHistory* scope_history,
												 vector<int>& curr_scope_context,
												 vector<int>& curr_node_context,
												 RunHelper& run_helper,
												 LoopFoldHistory* history) {
	int scope_id = scope_history->scope->id;
	curr_scope_context.push_back(scope_id);
	curr_node_context.push_back(-1);

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.find(scope_id);
	if (it == this->test_outer_state_networks.end()) {
		it = this->test_outer_state_networks.insert({scope_id, vector<vector<StateNetwork*>>()}).first;
	}

	int size_diff = (int)scope_history->scope->nodes.size() - (int)it->second.size();
	it->second.insert(it->second.begin(), size_diff, vector<StateNetwork*>());

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
				if (it->second[node_id].size() == 0) {
					for (int s_index = 0; s_index < this->test_num_new_outer_states; s_index++) {
						it->second[node_id].push_back(new StateNetwork(
							1,
							scope_history->scope->num_local_states,
							scope_history->scope->num_input_states,
							0,
							this->test_num_new_outer_states,
							20));
					}
				}
				history->outer_state_network_histories.push_back(vector<StateNetworkHistory*>());
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				for (int s_index = 0; s_index < this->test_num_new_outer_states; s_index++) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(it->second[node_id][s_index]);
					it->second[node_id][s_index]->new_outer_activate(
						action_node_history->obs_snapshot,
						action_node_history->ending_local_state_snapshot,
						action_node_history->ending_input_state_snapshot,
						new_outer_state_vals,
						state_network_history);
					history->outer_state_network_histories.back().push_back(state_network_history);
					new_outer_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				curr_node_context.back() = scope_history->node_histories[i_index][h_index]->scope_index;

				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				learn_outer_scope_activate_helper(new_outer_state_vals,
												  scope_node_history->inner_scope_history,
												  curr_scope_context,
												  curr_node_context,
												  run_helper,
												  history);

				curr_node_context.back() = -1;
			}
		}
	}

	curr_scope_context.pop_back();
	curr_node_context.pop_back();
}

void LoopFold::learn_inner_scope_activate_helper(vector<double>& new_state_vals,
												 ScopeHistory* scope_history,
												 vector<int>& curr_scope_context,
												 vector<int>& curr_node_context,
												 RunHelper& run_helper,
												 int iter_index,
												 int step_index,
												 LoopFoldHistory* history) {
	int scope_id = scope_history->scope->id;
	curr_scope_context.push_back(scope_id);
	curr_node_context.push_back(-1);

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_inner_state_networks.find(scope_id);
	if (it == this->test_inner_state_networks.end()) {
		it = this->test_inner_state_networks.insert({scope_id, vector<vector<StateNetwork*>>()}).first;
	}

	int size_diff = (int)scope_history->scope->nodes.size() - (int)it->second.size();
	it->second.insert(it->second.begin(), size_diff, vector<StateNetwork*>());

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
				if (it->second[node_id].size() == 0) {
					for (int s_index = 0; s_index < this->test_num_new_inner_states; s_index++) {
						it->second[node_id].push_back(new StateNetwork(
							1,
							scope_history->scope->num_local_states,
							scope_history->scope->num_input_states,
							0,
							this->test_num_new_inner_states,
							20));
					}
				}
				history->inner_state_network_histories[iter_index][step_index].push_back(vector<StateNetworkHistory*>());
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				for (int s_index = 0; s_index < this->test_num_new_inner_states; s_index++) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(it->second[node_id][s_index]);
					it->second[node_id][s_index]->new_outer_activate(
						action_node_history->obs_snapshot,
						action_node_history->ending_local_state_snapshot,
						action_node_history->ending_input_state_snapshot,
						new_state_vals,
						state_network_history);
					history->inner_state_network_histories[iter_index][step_index].back().push_back(state_network_history);
					new_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				curr_node_context.back() = scope_history->node_histories[i_index][h_index]->scope_index;

				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				learn_inner_scope_activate_helper(new_state_vals,
												  scope_node_history->inner_scope_history,
												  curr_scope_context,
												  curr_node_context,
												  run_helper,
												  iter_index,
												  step_index,
												  history);

				curr_node_context.back() = -1;
			}
		}
	}

	curr_scope_context.pop_back();
	curr_node_context.pop_back();
}

void LoopFold::learn_activate(vector<double>& local_state_vals,
							  vector<double>& input_vals,
							  vector<vector<double>>& flat_vals,
							  double& predicted_score,
							  double& scale_factor,
							  vector<ScopeHistory*>& context_histories,
							  RunHelper& run_helper,
							  LoopFoldHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT_LEARN;

	vector<double> new_outer_state_vals(this->test_num_new_outer_states, 0.0);

	ScopeHistory* scope_history = context_histories[context_histories.size() - this->scope_context.size()];
	vector<int> curr_scope_context;
	vector<int> curr_node_context;
	learn_outer_scope_activate_helper(new_outer_state_vals,
									  scope_history,
									  curr_scope_context,
									  curr_node_context,
									  run_helper,
									  history);

	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->test_num_new_inner_states, 0.0);

	history->starting_state_network_histories = vector<StateNetworkHistory*>(this->sum_inner_inputs + this->test_num_new_inner_states);
	for (int i_index = 0; i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
		StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_starting_state_networks[i_index]);
		this->test_starting_state_networks[i_index]->new_sequence_activate(
			new_inner_state_vals,
			local_state_vals,
			input_vals,
			new_outer_state_vals,
			state_network_history);
		history->starting_state_network_histories[i_index] = state_network_history;

		new_inner_state_vals[i_index] += this->test_starting_state_networks[i_index]->output->acti_vals[0];
	}

	geometric_distribution<int> distribution(0.5);
	int loop_iters = distribution(generator);
	history->num_loop_iters = loop_iters;

	for (int iter_index = 0; iter_index < loop_iters; iter_index++) {
		if (iter_index != loop_iters-1) {
			StateNetworkHistory* continue_score_network_history = new StateNetworkHistory(this->test_continue_score_network);
			this->test_continue_score_network->new_sequence_activate(
				new_inner_state_vals,
				local_state_vals,
				input_vals,
				new_outer_state_vals,
				continue_score_network_history);
			history->continue_score_network_updates.push_back(this->test_continue_score_network->output->acti_vals[0]);
			history->continue_score_network_histories.push_back(continue_score_network_history);

			predicted_score += scale_factor*this->test_continue_score_network->output->acti_vals[0];

			StateNetworkHistory* continue_misguess_network_history = new StateNetworkHistory(this->test_continue_misguess_network);
			this->test_continue_misguess_network->new_sequence_activate(
				new_inner_state_vals,
				local_state_vals,
				input_vals,
				new_outer_state_vals,
				continue_misguess_network_history);
			history->continue_misguess_vals.push_back(this->test_continue_misguess_network->output->acti_vals[0]);
			history->continue_misguess_network_histories.push_back(continue_misguess_network_history);
		} else {
			StateNetworkHistory* halt_score_network_history = new StateNetworkHistory(this->test_halt_score_network);
			this->test_halt_score_network->new_sequence_activate(
				new_inner_state_vals,
				local_state_vals,
				input_vals,
				new_outer_state_vals,
				halt_score_network_history);
			history->halt_score_network_update = this->test_halt_score_network->output->acti_vals[0];
			history->halt_score_network_history = halt_score_network_history;

			predicted_score += scale_factor*this->test_halt_score_network->output->acti_vals[0];

			StateNetworkHistory* halt_misguess_network_history = new StateNetworkHistory(this->test_halt_misguess_network);
			this->test_halt_misguess_network->new_sequence_activate(
				new_inner_state_vals,
				local_state_vals,
				input_vals,
				new_outer_state_vals,
				halt_misguess_network_history);
			history->halt_misguess_val = this->test_halt_misguess_network->output->acti_vals[0];
			history->halt_misguess_network_history = halt_misguess_network_history;
		}

		int num_inner_networks = this->sum_inner_inputs
			+ this->test_num_new_inner_states
			+ this->num_local_states
			+ this->num_input_states;
		history->state_network_histories.push_back(vector<vector<StateNetworkHistory*>>(
			this->sequence_length, vector<StateNetworkHistory*>(num_inner_networks, NULL)));
		history->inner_scope_histories.push_back(vector<ScopeHistory*>(this->sequence_length, NULL));
		history->score_network_updates.push_back(vector<double>(this->sequence_length));
		history->score_network_histories.push_back(vector<StateNetworkHistory*>(this->sequence_length, NULL));
		history->inner_state_network_histories.push_back(vector<vector<vector<StateNetworkHistory*>>>(this->sequence_length, vector<vector<StateNetworkHistory*>>()));

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_inner_scope[f_index]) {
				for (int i_index = 0; i_index < this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index++) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
					this->test_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[iter_index][f_index][i_index] = state_network_history;

					new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
				}

				Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
				vector<double> inner_input_vals(new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index],
					new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
				int num_input_states_diff = inner_scope->num_input_states - this->num_inner_inputs[f_index];
				inner_input_vals.insert(inner_input_vals.end(), num_input_states_diff, 0.0);

				// unused
				double inner_sum_impact = 0.0;
				vector<int> inner_scope_context;
				vector<int> inner_node_context;
				vector<ScopeHistory*> inner_context_histories;
				int inner_early_exit_depth;
				int inner_early_exit_node_id;
				FoldHistory* inner_early_exit_fold_history;
				int inner_explore_exit_depth;
				int inner_explore_exit_node_id;
				FoldHistory* inner_explore_exit_fold_history;

				ScopeHistory* scope_history = new ScopeHistory(inner_scope);
				inner_scope->activate(inner_input_vals,
									  flat_vals,
									  predicted_score,
									  scale_factor,
									  inner_sum_impact,
									  inner_scope_context,
									  inner_node_context,
									  inner_context_histories,
									  inner_early_exit_depth,
									  inner_early_exit_node_id,
									  inner_early_exit_fold_history,
									  inner_explore_exit_depth,
									  inner_explore_exit_node_id,
									  inner_explore_exit_fold_history,
									  run_helper,
									  scope_history);
				history->inner_scope_histories[iter_index][f_index] = scope_history;

				for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
					new_inner_state_vals[this->inner_input_start_indexes[f_index] + i_index] = inner_input_vals[i_index];
				}

				vector<double> new_state_vals;
				for (int i_index = 0; i_index < this->test_num_new_inner_states; i_index++) {
					new_state_vals.push_back(new_inner_state_vals[this->sum_inner_inputs+i_index]);
				}
				learn_inner_scope_activate_helper(new_state_vals,
												  scope_history,
												  inner_scope_context,	// empty again
												  inner_node_context,		// empty again
												  run_helper,
												  iter_index,
												  f_index,
												  history);
				for (int i_index = 0; i_index < this->test_num_new_inner_states; i_index++) {
					new_inner_state_vals[this->sum_inner_inputs+i_index] = new_state_vals[i_index];
				}

				// update back state so have chance to compress front after
				for (int i_index = this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index];
						i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
					this->test_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[iter_index][f_index][i_index] = state_network_history;

					new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
				}
				for (int l_index = 0; l_index < this->num_local_states; l_index++) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ l_index;
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
					this->test_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[iter_index][f_index][state_index] = state_network_history;

					local_state_vals[l_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
				}
				for (int i_index = 0; i_index < this->num_input_states; i_index++) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ this->num_local_states
						+ i_index;
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
					this->test_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[iter_index][f_index][state_index] = state_network_history;

					input_vals[i_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
				}
			} else {
				double obs = (*flat_vals.begin())[0];

				for (int i_index = 0; i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
					this->test_state_networks[f_index][i_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[iter_index][f_index][i_index] = state_network_history;

					new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
				}
				for (int l_index = 0; l_index < this->num_local_states; l_index++) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ l_index;
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
					this->test_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[iter_index][f_index][state_index] = state_network_history;

					local_state_vals[l_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
				}
				for (int i_index = 0; i_index < this->num_input_states; i_index++) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ this->num_local_states
						+ i_index;
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
					this->test_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[iter_index][f_index][state_index] = state_network_history;

					input_vals[i_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
				}

				flat_vals.erase(flat_vals.begin());
			}

			StateNetworkHistory* score_network_history = new StateNetworkHistory(this->test_score_networks[f_index]);
			this->test_score_networks[f_index]->new_sequence_activate(
				new_inner_state_vals,
				local_state_vals,
				input_vals,
				new_outer_state_vals,
				score_network_history);
			history->score_network_updates[iter_index][f_index] = this->test_score_networks[f_index]->output->acti_vals[0];
			history->score_network_histories[iter_index][f_index] = score_network_history;
			predicted_score += scale_factor*this->test_score_networks[f_index]->output->acti_vals[0];
		}
	}
}

void LoopFold::learn_backprop(vector<double>& local_state_errors,
							  vector<double>& input_errors,
							  double target_val,
							  double final_misguess,
							  double& predicted_score,
							  double& scale_factor,
							  RunHelper& run_helper,
							  LoopFoldHistory* history) {
	this->test_average_score = 0.9999*this->test_average_score + 0.0001*target_val;
	double score_variance = (this->test_average_score - target_val)*(this->test_average_score - target_val);
	this->test_score_variance = 0.9999*this->test_score_variance + 0.0001*score_variance;
	this->test_average_misguess = 0.9999*this->test_average_misguess + 0.0001*final_misguess;
	double misguess_variance = (this->test_average_misguess - final_misguess)*(this->test_average_misguess - final_misguess);
	this->test_misguess_variance = 0.9999*this->test_misguess_variance + 0.0001*misguess_variance;

	this->sum_error += abs(target_val-predicted_score);

	vector<double> new_inner_state_errors(this->sum_inner_inputs+this->test_num_new_inner_states, 0.0);
	vector<double> new_outer_state_errors(this->test_num_new_outer_states, 0.0);

	for (int iter_index = history->num_loop_iters-1; iter_index >= 0; iter_index--) {
		for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
			double score_network_target_max_update;
			if (this->state == FOLD_STATE_EXPLORE && this->state_iter <= 100000) {
				score_network_target_max_update = 0.05;
			} else if (this->state_iter <= 400000) {
				score_network_target_max_update = 0.01;
			} else {
				score_network_target_max_update = 0.002;
			}
			this->test_score_networks[f_index]->new_sequence_backprop(
				target_val - predicted_score,
				new_inner_state_errors,
				local_state_errors,
				input_errors,
				new_outer_state_errors,
				score_network_target_max_update,
				history->score_network_histories[iter_index][f_index]);

			predicted_score -= scale_factor*history->score_network_updates[iter_index][f_index];

			if (this->is_inner_scope[f_index]) {
				for (int i_index = this->num_input_states-1; i_index >= 0; i_index--) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ this->num_local_states
						+ i_index;
					// TODO: tune max_updates due to recurrent
					double state_network_target_max_update;
					if (this->state == FOLD_STATE_EXPLORE && this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.01;
					} else {
						state_network_target_max_update = 0.002;
					}
					this->test_state_networks[f_index][state_index]->new_sequence_backprop(
						input_errors[i_index],
						new_inner_state_errors,
						local_state_errors,
						input_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[iter_index][f_index][state_index]);
				}
				for (int l_index = this->num_local_states-1; l_index >= 0; l_index--) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ l_index;
					double state_network_target_max_update;
					if (this->state == FOLD_STATE_EXPLORE && this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.01;
					} else {
						state_network_target_max_update = 0.002;
					}
					this->test_state_networks[f_index][state_index]->new_sequence_backprop(
						local_state_errors[l_index],
						new_inner_state_errors,
						local_state_errors,
						input_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[iter_index][f_index][state_index]);
				}
				for (int i_index = this->sum_inner_inputs+this->test_num_new_inner_states-1;
						i_index >= this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index--) {
					double state_network_target_max_update;
					if ((this->state == FOLD_STATE_EXPLORE || (this->state == FOLD_STATE_ADD_INNER_STATE && i_index == this->sum_inner_inputs+this->test_num_new_inner_states-1))
							&& this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.01;
					} else {
						state_network_target_max_update = 0.002;
					}
					this->test_state_networks[f_index][i_index]->new_sequence_backprop(
						new_inner_state_errors[i_index],
						new_inner_state_errors,
						local_state_errors,
						input_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[iter_index][f_index][i_index]);
				}

				Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];

				vector<double> new_state_errors;
				for (int i_index = 0; i_index < this->test_num_new_inner_states; i_index++) {
					new_state_errors.push_back(new_inner_state_errors[this->sum_inner_inputs+i_index]);
				}
				for (int n_index = (int)history->inner_state_network_histories[iter_index][f_index].size()-1; n_index >= 0; n_index--) {
					for (int i_index = this->test_num_new_inner_states-1; i_index >= 0; i_index--) {
						StateNetwork* state_network = history->inner_state_network_histories[iter_index][f_index][n_index][i_index]->network;
						double state_network_target_max_update;
						if ((this->state == FOLD_STATE_EXPLORE || (this->state == FOLD_STATE_ADD_INNER_STATE && i_index == this->test_num_new_inner_states-1))
								&& this->state_iter <= 100000) {
							state_network_target_max_update = 0.05;
						} else if (this->state_iter <= 400000) {
							state_network_target_max_update = 0.01;
						} else {
							state_network_target_max_update = 0.002;
						}
						state_network->new_outer_backprop(
							new_state_errors[i_index],
							new_state_errors,
							state_network_target_max_update,
							history->inner_state_network_histories[iter_index][f_index][n_index][i_index]);
					}
				}
				for (int i_index = 0; i_index < this->test_num_new_inner_states; i_index++) {
					new_inner_state_errors[this->sum_inner_inputs+i_index] = new_state_errors[i_index];
				}

				vector<double> inner_input_errors(new_inner_state_errors.begin() + this->inner_input_start_indexes[f_index],
					new_inner_state_errors.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
				int num_input_states_diff = inner_scope->num_input_states - this->num_inner_inputs[f_index];
				inner_input_errors.insert(inner_input_errors.end(), num_input_states_diff, 0.0);
				
				// unused
				double inner_final_misguess = 0.0;
				double inner_final_sum_impact = 0.0;

				inner_scope->backprop(inner_input_errors,
									  target_val,
									  inner_final_misguess,
									  inner_final_sum_impact,
									  predicted_score,
									  scale_factor,
									  run_helper,
									  history->inner_scope_histories[iter_index][f_index]);

				for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
					new_inner_state_errors[this->inner_input_start_indexes[f_index] + i_index] = inner_input_errors[i_index];
				}

				for (int i_index = this->inner_input_start_indexes[f_index]+this->num_inner_inputs[f_index]-1; i_index >= 0; i_index--) {
					double state_network_target_max_update;
					if (this->state == FOLD_STATE_EXPLORE && this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.01;
					} else {
						state_network_target_max_update = 0.002;
					}
					this->test_state_networks[f_index][i_index]->new_sequence_backprop(
						new_inner_state_errors[i_index],
						new_inner_state_errors,
						local_state_errors,
						input_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[iter_index][f_index][i_index]);
				}
			} else {
				for (int i_index = this->num_input_states-1; i_index >= 0; i_index--) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ this->num_local_states
						+ i_index;
					double state_network_target_max_update;
					if (this->state == FOLD_STATE_EXPLORE && this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.01;
					} else {
						state_network_target_max_update = 0.002;
					}
					this->test_state_networks[f_index][state_index]->new_sequence_backprop(
						input_errors[i_index],
						new_inner_state_errors,
						local_state_errors,
						input_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[iter_index][f_index][state_index]);
				}
				for (int l_index = this->num_local_states-1; l_index >= 0; l_index--) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ l_index;
					double state_network_target_max_update;
					if (this->state == FOLD_STATE_EXPLORE && this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.01;
					} else {
						state_network_target_max_update = 0.002;
					}
					this->test_state_networks[f_index][state_index]->new_sequence_backprop(
						local_state_errors[l_index],
						new_inner_state_errors,
						local_state_errors,
						input_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[iter_index][f_index][state_index]);
				}
				for (int i_index = this->sum_inner_inputs+this->test_num_new_inner_states-1; i_index >= 0; i_index--) {
					double state_network_target_max_update;
					if ((this->state == FOLD_STATE_EXPLORE || (this->state == FOLD_STATE_ADD_INNER_STATE && i_index == this->sum_inner_inputs+this->test_num_new_inner_states-1))
							&& this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.01;
					} else {
						state_network_target_max_update = 0.002;
					}
					this->test_state_networks[f_index][i_index]->new_sequence_backprop(
						new_inner_state_errors[i_index],
						new_inner_state_errors,
						local_state_errors,
						input_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[iter_index][f_index][i_index]);
				}
			}
		}

		if (iter_index != history->num_loop_iters-1) {
			double score_network_target_max_update;
			if (this->state == FOLD_STATE_EXPLORE && this->state_iter <= 100000) {
				score_network_target_max_update = 0.05;
			} else if (this->state_iter <= 400000) {
				score_network_target_max_update = 0.01;
			} else {
				score_network_target_max_update = 0.002;
			}
			this->test_continue_score_network->new_sequence_backprop(
				target_val - predicted_score,
				new_inner_state_errors,
				local_state_errors,
				input_errors,
				new_outer_state_errors,
				score_network_target_max_update,
				history->continue_score_network_histories[iter_index]);

			predicted_score -= scale_factor*history->continue_score_network_updates[iter_index];

			double misguess_network_target_max_update;
			if (this->state == FOLD_STATE_EXPLORE && this->state_iter <= 100000) {
				misguess_network_target_max_update = 0.05;
			} else if (this->state_iter <= 400000) {
				misguess_network_target_max_update = 0.01;
			} else {
				misguess_network_target_max_update = 0.002;
			}
			this->test_continue_misguess_network->new_sequence_backprop(
				final_misguess - history->continue_misguess_vals[iter_index],
				new_inner_state_errors,
				local_state_errors,
				input_errors,
				new_outer_state_errors,
				misguess_network_target_max_update,
				history->continue_misguess_network_histories[iter_index]);
		} else {
			double score_network_target_max_update;
			if (this->state == FOLD_STATE_EXPLORE && this->state_iter <= 100000) {
				score_network_target_max_update = 0.05;
			} else if (this->state_iter <= 400000) {
				score_network_target_max_update = 0.01;
			} else {
				score_network_target_max_update = 0.002;
			}
			this->test_halt_score_network->new_sequence_backprop(
				target_val - predicted_score,
				new_inner_state_errors,
				local_state_errors,
				input_errors,
				new_outer_state_errors,
				score_network_target_max_update,
				history->halt_score_network_history);

			predicted_score -= scale_factor*history->halt_score_network_update;

			double misguess_network_target_max_update;
			if (this->state == FOLD_STATE_EXPLORE && this->state_iter <= 100000) {
				misguess_network_target_max_update = 0.05;
			} else if (this->state_iter <= 400000) {
				misguess_network_target_max_update = 0.01;
			} else {
				misguess_network_target_max_update = 0.002;
			}
			this->test_halt_misguess_network->new_sequence_backprop(
				final_misguess - history->halt_misguess_val,
				new_inner_state_errors,
				local_state_errors,
				input_errors,
				new_outer_state_errors,
				misguess_network_target_max_update,
				history->halt_misguess_network_history);
		}
	}

	for (int i_index = this->sum_inner_inputs+this->test_num_new_inner_states-1; i_index >= 0; i_index--) {
		double state_network_target_max_update;
		if ((this->state == FOLD_STATE_EXPLORE || (this->state == FOLD_STATE_ADD_INNER_STATE && i_index == this->sum_inner_inputs+this->test_num_new_inner_states-1))
				&& this->state_iter <= 100000) {
			state_network_target_max_update = 0.05;
		} else if (this->state_iter <= 400000) {
			state_network_target_max_update = 0.01;
		} else {
			state_network_target_max_update = 0.002;
		}
		this->test_starting_state_networks[i_index]->new_sequence_backprop(
			new_inner_state_errors[i_index],
			new_inner_state_errors,
			local_state_errors,
			input_errors,
			new_outer_state_errors,
			state_network_target_max_update,
			history->starting_state_network_histories[i_index]);
	}

	for (int n_index = (int)history->outer_state_network_histories.size()-1; n_index >= 0; n_index--) {
		for (int o_index = this->test_num_new_outer_states-1; o_index >= 0; o_index--) {
			StateNetwork* state_network = history->outer_state_network_histories[n_index][o_index]->network;
			double state_network_target_max_update;
			if ((this->state == FOLD_STATE_EXPLORE || (this->state == FOLD_STATE_ADD_OUTER_STATE && o_index == this->test_num_new_outer_states-1))
					&& this->state_iter <= 100000) {
				state_network_target_max_update = 0.05;
			} else if (this->state_iter <= 400000) {
				state_network_target_max_update = 0.01;
			} else {
				state_network_target_max_update = 0.002;
			}
			state_network->new_outer_backprop(
				new_outer_state_errors[o_index],
				new_outer_state_errors,
				state_network_target_max_update,
				history->outer_state_network_histories[n_index][o_index]);
		}
	}
}
