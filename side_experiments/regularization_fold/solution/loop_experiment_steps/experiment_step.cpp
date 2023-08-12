#include "loop_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "action_node.h"
#include "constants.h"
#include "exit_network.h"
#include "globals.h"
#include "layer.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "sequence.h"
#include "state_network.h"

using namespace std;

void LoopExperiment::experiment_pre_activate_helper(
		bool on_path,
		int context_index,
		double& temp_scale_factor,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator state_it = this->state_networks.find(scope_id);
	map<int, vector<ScoreNetwork*>>::iterator score_it = this->score_networks.find(scope_id);

	if (state_it == this->state_networks.end()) {
		state_it = this->state_networks.insert({scope_id, vector<vector<StateNetwork*>>()}).first;
		score_it = this->score_networks.insert({scope_id, vector<ScoreNetwork*>()}).first;

		this->scope_furthest_layer_seen_in[scope_id] = context_index;
		this->scope_steps_seen_in[scope_id] = vector<bool>(1, false);
	}

	int size_diff = (int)scope_history->scope->nodes.size() - (int)state_it->second.size();
	state_it->second.insert(state_it->second.end(), size_diff, vector<StateNetwork*>());
	score_it->second.insert(score_it->second.end(), size_diff, NULL);

	map<int, int>::iterator seen_it = this->scope_furthest_layer_seen_in.find(scope_id);
	if (context_index < seen_it->second) {
		seen_it->second = context_index;

		int new_furthest_distance = (int)this->scope_context.size()+2 - context_index;
		for (int n_index = 0; n_index < (int)state_it->second.size(); n_index++) {
			if (state_it->second[n_index].size() != 0) {
				for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
					state_it->second[n_index][s_index]->update_lasso_weights(new_furthest_distance);
				}
				score_it->second[n_index]->update_lasso_weights(new_furthest_distance);
			}
		}
	}

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->node->id;

				if (state_it->second[node_id].size() == 0) {
					int new_furthest_distance = (int)this->scope_context.size()+2 - seen_it->second;
					for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
						state_it->second[node_id].push_back(
							new StateNetwork(scope_history->scope->num_states,
											 NUM_NEW_STATES,
											 0,
											 20));
						state_it->second[node_id].back()->update_lasso_weights(new_furthest_distance);
					}
					score_it->second[node_id] = new ScoreNetwork(scope_history->scope->num_states,
																 NUM_NEW_STATES,
																 20);
					score_it->second[node_id]->update_lasso_weights(new_furthest_distance);
				}

				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

				action_node_history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

				action_node_history->new_state_network_histories = vector<StateNetworkHistory*>(NUM_NEW_STATES, NULL);
				for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
					if (run_helper.can_zero && rand()%5 == 0) {
						// do nothing
					} else {
						StateNetwork* network = state_it->second[node_id][s_index];
						StateNetworkHistory* network_history = new StateNetworkHistory(network);
						network->new_activate(action_node_history->obs_snapshot,
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
				score_network->new_activate(action_node_history->ending_state_vals_snapshot,
											action_node_history->ending_new_state_vals_snapshot,
											score_network_history);
				action_node_history->new_score_network_history = score_network_history;
				action_node_history->new_score_network_output = score_network->output->acti_vals[0];

				run_helper.predicted_score += temp_scale_factor*score_network->output->acti_vals[0];
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				temp_scale_factor *= scope_node->scope_scale_mod->weight;

				if (on_path
						&& i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					experiment_pre_activate_helper(false,
												   context_index,
												   temp_scale_factor,
												   run_helper,
												   scope_node_history->inner_scope_history);

					temp_scale_factor /= scope_node->scope_scale_mod->weight;
				}
			}
		}
	}
}

void LoopExperiment::experiment_activate(vector<double>& flat_vals,
										 vector<ForwardContextLayer>& context,
										 RunHelper& run_helper,
										 LoopExperimentHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT;

	run_helper.experiment = this;
	if (rand()%5 == 0) {
		run_helper.can_zero = true;
	} else {
		run_helper.can_zero = false;
	}
	run_helper.new_state_vals = vector<double>(NUM_NEW_STATES, 0.0);

	double temp_scale_factor = 1.0;
	int context_size_diff = (int)context.size() - (int)this->scope_context.size() - 1;
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		int context_index = c_index - context_size_diff;
		if (context_index < 0) {
			context_index = 0;
		}
		experiment_pre_activate_helper(true,
									   context_index,
									   temp_scale_factor,
									   run_helper,
									   context[c_index].scope_history);
	}

	int loop_iters = rand()%6;

	run_helper.experiment_on_path = false;
	run_helper.experiment_context_index = (int)this->scope_context.size()+1;
	run_helper.experiment_step_index = 0;

	vector<double> input_vals(this->sequence->input_types.size(), 0.0);
	vector<vector<double>> empty_previous_vals;
	this->sequence->activate_pull(input_vals,
								  context,
								  empty_previous_vals,
								  NULL,
								  run_helper);

	for (int i_index = 0; i_index < loop_iters; i_index++) {
		history->iter_input_vals_snapshots.push_back(input_vals);
		history->iter_new_state_vals_snapshots.push_back(run_helper.new_state_vals);

		ScoreNetworkHistory* continue_score_network_history = new ScoreNetworkHistory(this->continue_score_network);
		this->continue_score_network->new_activate(input_vals,
												   run_helper.new_state_vals,
												   continue_score_network_history);
		history->continue_score_network_histories.push_back(continue_score_network_history);
		history->continue_score_network_outputs.push_back(this->continue_score_network->output->acti_vals[0]);

		this->halt_score_network->new_activate(input_vals,
											   run_helper.new_state_vals);
		history->halt_score_snapshots.push_back(
			run_helper.predicted_score + run_helper.scale_factor*this->halt_score_network->output->acti_vals[0]);

		ScoreNetworkHistory* continue_misguess_network_history = new ScoreNetworkHistory(this->continue_misguess_network);
		this->continue_misguess_network->new_activate(input_vals,
													  run_helper.new_state_vals,
													  continue_misguess_network_history);
		history->continue_misguess_network_histories.push_back(continue_misguess_network_history);
		history->continue_misguess_network_outputs.push_back(this->continue_misguess_network->output->acti_vals[0]);

		this->halt_misguess_network->new_activate(input_vals,
												  run_helper.new_state_vals);
		history->halt_misguess_snapshots.push_back(this->halt_misguess_network->output->acti_vals[0]);

		run_helper.scale_factor *= this->scale_mod->weight;

		SequenceHistory* sequence_history = new SequenceHistory(this->sequence);
		history->sequence_histories.push_back(sequence_history);
		this->sequence->activate(input_vals,
								 flat_vals,
								 run_helper,
								 sequence_history);

		run_helper.scale_factor /= this->scale_mod->weight;
	}

	history->ending_input_vals_snapshot = input_vals;
	history->ending_new_state_vals_snapshot = run_helper.new_state_vals;

	ScoreNetworkHistory* halt_score_network_history = new ScoreNetworkHistory(this->halt_score_network);
	this->halt_score_network->new_activate(history->ending_input_vals_snapshot,
										   history->ending_new_state_vals_snapshot,
										   halt_score_network_history);
	history->halt_score_network_history = halt_score_network_history;
	history->halt_score_network_output = this->halt_score_network->output->acti_vals[0];

	ScoreNetworkHistory* halt_misguess_network_history = new ScoreNetworkHistory(this->halt_misguess_network);
	this->halt_misguess_network->new_activate(history->ending_input_vals_snapshot,
											  history->ending_new_state_vals_snapshot,
											  halt_misguess_network_history);
	history->halt_misguess_network_history = halt_misguess_network_history;
	history->halt_misguess_network_output = this->halt_misguess_network->output->acti_vals[0];

	this->sequence->activate_reset(input_vals,
								   context,
								   empty_previous_vals);

	history->exit_state_vals_snapshot = vector<vector<double>>(1);
	history->exit_state_vals_snapshot[0] = *(context.back().state_vals);

	history->exit_network_histories = vector<ExitNetworkHistory*>(this->exit_networks.size(), NULL);
	for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
		if (context.back().states_initialized[s_index]) {
			ExitNetworkHistory* network_history = new ExitNetworkHistory(this->exit_networks[s_index]);
			this->exit_networks[s_index]->new_activate(history->exit_state_vals_snapshot,
													   history->ending_new_state_vals_snapshot,
													   network_history);
			history->exit_network_histories[s_index] = network_history;
			context.back().state_vals->at(s_index) += this->exit_networks[s_index]->output->acti_vals[0];
		}
	}

	run_helper.experiment_on_path = true;
	run_helper.experiment_context_index--;
	run_helper.experiment_step_index = -1;
}

void LoopExperiment::experiment_backprop(vector<BackwardContextLayer>& context,
										 RunHelper& run_helper,
										 LoopExperimentHistory* history) {
	for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
		if (history->exit_network_histories[s_index] != NULL) {
			double exit_network_target_max_update;
			if (this->state_iter <= 100000) {
				exit_network_target_max_update = 0.05;
			} else {
				exit_network_target_max_update = 0.01;
			}
			if (this->state_iter <= 200000) {
				this->exit_networks[s_index]->new_scaled_backprop(context.back().state_errors->at(s_index),
																  run_helper.new_state_errors,
																  exit_network_target_max_update,
																  history->exit_state_vals_snapshot,
																  history->ending_new_state_vals_snapshot,
																  history->exit_network_histories[s_index]);
			} else {
				this->exit_networks[s_index]->new_lasso_backprop(context.back().state_errors->at(s_index),
																 run_helper.new_state_errors,
																 exit_network_target_max_update,
																 history->exit_state_vals_snapshot,
																 history->ending_new_state_vals_snapshot,
																 history->exit_network_histories[s_index]);
			}
		}
	}

	this->sum_error += abs(run_helper.target_val - run_helper.predicted_score);

	vector<double> input_errors(this->sequence->input_types.size(), 0.0);
	vector<vector<double>> empty_previous_errors;
	this->sequence->backprop_pull(input_errors,
								  context,
								  empty_previous_errors);

	double halt_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->halt_score_network_output;
	double halt_predicted_score_error = run_helper.target_val - halt_predicted_score;
	double halt_score_network_target_max_update;
	if (this->state_iter <= 100000) {
		halt_score_network_target_max_update = 0.05;
	} else {
		halt_score_network_target_max_update = 0.01;
	}
	if (this->state_iter <= 200000) {
		this->halt_score_network->new_scaled_backprop(
			run_helper.scale_factor*halt_predicted_score_error,
			run_helper.new_state_errors,
			halt_score_network_target_max_update,
			history->ending_input_vals_snapshot,
			history->ending_new_state_vals_snapshot,
			history->halt_score_network_history);
	} else {
		this->halt_score_network->new_lasso_backprop(
			run_helper.scale_factor*halt_predicted_score_error,
			run_helper.new_state_errors,
			halt_score_network_target_max_update,
			history->ending_input_vals_snapshot,
			history->ending_new_state_vals_snapshot,
			history->halt_score_network_history);
	}

	double halt_misguess_error = run_helper.final_misguess - history->halt_misguess_network_output;
	double halt_misguess_network_target_max_update;
	if (this->state_iter <= 100000) {
		halt_misguess_network_target_max_update = 0.05;
	} else {
		halt_misguess_network_target_max_update = 0.01;
	}
	if (this->state_iter <= 200000) {
		this->halt_misguess_network->new_scaled_backprop(
			halt_misguess_error,
			run_helper.new_state_errors,
			halt_misguess_network_target_max_update,
			history->ending_input_vals_snapshot,
			history->ending_new_state_vals_snapshot,
			history->halt_misguess_network_history);
	} else {
		this->halt_misguess_network->new_lasso_backprop(
			halt_misguess_error,
			run_helper.new_state_errors,
			halt_misguess_network_target_max_update,
			history->ending_input_vals_snapshot,
			history->ending_new_state_vals_snapshot,
			history->halt_misguess_network_history);
	}

	for (int i_index = (int)history->sequence_histories.size()-1; i_index >= 0; i_index--) {
		run_helper.scale_factor *= this->scale_mod->weight;

		double inner_scale_factor_error = 0.0;

		this->sequence->backprop(input_errors,
								 inner_scale_factor_error,
								 run_helper,
								 history->sequence_histories[i_index]);

		this->scale_mod->backprop(inner_scale_factor_error, 0.002);

		run_helper.scale_factor /= this->scale_mod->weight;

		double best_halt_score = run_helper.target_val;
		double best_halt_misguess = run_helper.final_misguess;
		// back to front
		for (int ii_index = (int)history->sequence_histories.size()-1; ii_index >= i_index+1; ii_index--) {
			double score_diff = history->halt_score_snapshots[ii_index] - best_halt_score;
			double score_val = score_diff / (solution->average_misguess*abs(run_helper.scale_factor));
			if (score_val > 0.1) {
				best_halt_score = history->halt_score_snapshots[ii_index];
				best_halt_misguess = history->halt_misguess_snapshots[ii_index];
			} else if (score_val < -0.1) {
				continue;
			} else {
				double misguess_diff = history->halt_misguess_snapshots[ii_index] - best_halt_misguess;
				double misguess_val = misguess_diff / (solution->misguess_standard_deviation*abs(run_helper.scale_factor));
				if (misguess_val < -0.1) {
					best_halt_score = history->halt_score_snapshots[ii_index];
					best_halt_misguess = history->halt_misguess_snapshots[ii_index];
				} else if (misguess_val > 0.1) {
					continue;
				} else {
					// use earlier iter if no strong signal either way
					best_halt_score = history->halt_score_snapshots[ii_index];
					best_halt_misguess = history->halt_misguess_snapshots[ii_index];
				}
			}
		}

		double continue_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->continue_score_network_outputs[i_index];
		double continue_predicted_score_error = best_halt_score - continue_predicted_score;
		double continue_score_network_target_max_update;
		if (this->state_iter <= 100000) {
			continue_score_network_target_max_update = 0.05;
		} else {
			continue_score_network_target_max_update = 0.01;
		}
		if (this->state_iter <= 200000) {
			this->continue_score_network->new_scaled_backprop(
				run_helper.scale_factor*continue_predicted_score_error,
				run_helper.new_state_errors,
				continue_score_network_target_max_update,
				history->iter_input_vals_snapshots[i_index],
				history->iter_new_state_vals_snapshots[i_index],
				history->continue_score_network_histories[i_index]);
		} else {
			this->continue_score_network->new_lasso_backprop(
				run_helper.scale_factor*continue_predicted_score_error,
				run_helper.new_state_errors,
				continue_score_network_target_max_update,
				history->iter_input_vals_snapshots[i_index],
				history->iter_new_state_vals_snapshots[i_index],
				history->continue_score_network_histories[i_index]);
		}

		double continue_misguess_error = best_halt_misguess - history->continue_misguess_network_outputs[i_index];
		double continue_misguess_network_target_max_update;
		if (this->state_iter <= 100000) {
			continue_misguess_network_target_max_update = 0.05;
		} else {
			continue_misguess_network_target_max_update = 0.01;
		}
		if (this->state_iter <= 200000) {
			this->continue_misguess_network->new_scaled_backprop(
				continue_misguess_error,
				run_helper.new_state_errors,
				continue_misguess_network_target_max_update,
				history->iter_input_vals_snapshots[i_index],
				history->iter_new_state_vals_snapshots[i_index],
				history->continue_misguess_network_histories[i_index]);
		} else {
			this->continue_misguess_network->new_lasso_backprop(
				continue_misguess_error,
				run_helper.new_state_errors,
				continue_misguess_network_target_max_update,
				history->iter_input_vals_snapshots[i_index],
				history->iter_new_state_vals_snapshots[i_index],
				history->continue_misguess_network_histories[i_index]);
		}
	}

	this->sequence->backprop_reset(input_errors,
								   context,
								   empty_previous_errors);
	run_helper.new_input_errors.push_back(input_errors);

	run_helper.backprop_is_pre_experiment = true;
}
