#include "branch_experiment.h"

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

void BranchExperiment::experiment_pre_activate_helper(
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
		this->scope_steps_seen_in[scope_id] = vector<bool>(this->num_steps, false);
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

void BranchExperiment::experiment_activate(vector<double>& flat_vals,
										   vector<ForwardContextLayer>& context,
										   RunHelper& run_helper,
										   BranchExperimentHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT;

	history->existing_predicted_score = run_helper.predicted_score;

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

	history->starting_state_vals_snapshot = *(context.back().state_vals);
	history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

	// no need to activate starting networks until backprop

	// no need to append to context yet

	history->step_obs_snapshots = vector<double>(this->num_steps, 0.0);
	history->step_starting_new_state_vals_snapshots = vector<vector<double>>(this->num_steps);
	history->step_state_network_histories = vector<vector<StateNetworkHistory*>>(this->num_steps);
	history->step_ending_new_state_vals_snapshots = vector<vector<double>>(this->num_steps);
	history->step_score_network_histories = vector<ScoreNetworkHistory*>(this->num_steps, NULL);
	history->step_score_network_outputs = vector<double>(this->num_steps, 0.0);
	history->step_input_sequence_step_indexes = vector<vector<int>>(this->num_steps);
	history->step_input_vals_snapshots = vector<vector<vector<double>>>(this->num_steps);
	history->step_input_state_network_histories = vector<vector<vector<StateNetworkHistory*>>>(this->num_steps);

	history->sequence_histories = vector<SequenceHistory*>(this->num_steps, NULL);

	run_helper.experiment_on_path = false;
	run_helper.experiment_context_index = (int)this->scope_context.size()+1;

	vector<vector<double>> sequence_input_vals(this->num_steps);
	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
			double obs = flat_vals[0];

			history->step_obs_snapshots[a_index] = obs;
			history->step_starting_new_state_vals_snapshots[a_index] = run_helper.new_state_vals;

			history->step_state_network_histories[a_index] = vector<StateNetworkHistory*>(NUM_NEW_STATES, NULL);
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				if (run_helper.can_zero && rand()%5 == 0) {
					// do nothing
				} else {
					StateNetwork* network = this->step_state_networks[a_index][s_index];
					vector<double> empty_state_vals;
					StateNetworkHistory* network_history = new StateNetworkHistory(network);
					network->new_activate(history->step_obs_snapshots[a_index],
										  empty_state_vals,
										  history->step_starting_new_state_vals_snapshots[a_index],
										  network_history);
					history->step_state_network_histories[a_index][s_index] = network_history;
					run_helper.new_state_vals[s_index] += network->output->acti_vals[0];
				}
			}

			history->step_ending_new_state_vals_snapshots[a_index] = run_helper.new_state_vals;

			ScoreNetwork* score_network = this->step_score_networks[a_index];
			vector<double> empty_state_vals;
			ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(score_network);
			score_network->new_activate(empty_state_vals,
										history->step_ending_new_state_vals_snapshots[a_index],
										score_network_history);
			history->step_score_network_histories[a_index] = score_network_history;
			history->step_score_network_outputs[a_index] = score_network->output->acti_vals[0];

			run_helper.predicted_score += run_helper.scale_factor*score_network->output->acti_vals[0];
			if (this->state_iter%1000 == 0) {
				cout << "score_network->output->acti_vals[0]: " << score_network->output->acti_vals[0] << endl;
			}

			flat_vals.erase(flat_vals.begin());
		} else {
			sequence_input_vals[a_index] = vector<double>(this->sequences[a_index]->input_types.size(), 0.0);
			this->sequences[a_index]->activate_pull(sequence_input_vals[a_index],
													context,
													sequence_input_vals,
													history,
													run_helper);

			run_helper.scale_factor *= this->sequence_scale_mods[a_index]->weight;

			run_helper.experiment_step_index = a_index;

			SequenceHistory* sequence_history = new SequenceHistory(this->sequences[a_index]);
			history->sequence_histories[a_index] = sequence_history;
			this->sequences[a_index]->activate(sequence_input_vals[a_index],
											   flat_vals,
											   run_helper,
											   sequence_history);

			run_helper.scale_factor /= this->sequence_scale_mods[a_index]->weight;
		}
	}

	for (int a_index = this->num_steps-1; a_index >= 0; a_index--) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
			this->sequences[a_index]->activate_reset(sequence_input_vals[a_index],
													 context,
													 sequence_input_vals);
		}
	}

	history->exit_state_vals_snapshot = vector<vector<double>>(this->exit_depth+1);
	for (int l_index = 0; l_index < this->exit_depth+1; l_index++) {
		history->exit_state_vals_snapshot[l_index] = *(context[
			context.size() - (this->exit_depth+1) + l_index].state_vals);
	}
	history->ending_new_state_vals_snapshot = run_helper.new_state_vals;

	history->exit_network_histories = vector<ExitNetworkHistory*>(this->exit_networks.size(), NULL);
	for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
		if (context[context.size() - (this->exit_depth+1)].states_initialized[s_index]) {
			ExitNetworkHistory* network_history = new ExitNetworkHistory(this->exit_networks[s_index]);
			this->exit_networks[s_index]->new_activate(history->exit_state_vals_snapshot,
													   history->ending_new_state_vals_snapshot,
													   network_history);
			history->exit_network_histories[s_index] = network_history;
			context[context.size() - (this->exit_depth+1)].state_vals->at(s_index)
				+= this->exit_networks[s_index]->output->acti_vals[0];
		}
	}

	run_helper.experiment_on_path = true;
	run_helper.experiment_context_index--;
	// run_helper.experiment_context_index still has to be >0
	run_helper.experiment_step_index = -1;
}

void BranchExperiment::experiment_backprop(vector<BackwardContextLayer>& context,
										   RunHelper& run_helper,
										   BranchExperimentHistory* history) {
	for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
		if (history->exit_network_histories[s_index] != NULL) {
			double exit_network_target_max_update;
			if (this->state_iter <= 100000) {
				exit_network_target_max_update = 0.05;
			} else {
				exit_network_target_max_update = 0.01;
			}
			if (this->state_iter <= 300000) {
				this->exit_networks[s_index]->new_scaled_backprop(context[context.size() - (this->exit_depth+1)].state_errors->at(s_index),
																  run_helper.new_state_errors,
																  exit_network_target_max_update,
																  history->exit_state_vals_snapshot,
																  history->ending_new_state_vals_snapshot,
																  history->exit_network_histories[s_index]);
			} else {
				this->exit_networks[s_index]->new_lasso_backprop(context[context.size() - (this->exit_depth+1)].state_errors->at(s_index),
																 run_helper.new_state_errors,
																 exit_network_target_max_update,
																 history->exit_state_vals_snapshot,
																 history->ending_new_state_vals_snapshot,
																 history->exit_network_histories[s_index]);
			}
		}
	}

	this->sum_error += abs(run_helper.target_val - run_helper.predicted_score);

	// no need to append to context yet

	run_helper.new_input_errors = vector<vector<double>>(this->num_steps);

	vector<vector<double>> sequence_input_errors(this->num_steps);
	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
			sequence_input_errors[a_index] = vector<double>(this->sequences[a_index]->input_types.size(), 0.0);
			this->sequences[a_index]->backprop_pull(sequence_input_errors[a_index],
													context,
													sequence_input_errors);
		}
	}

	for (int a_index = this->num_steps-1; a_index >= 0; a_index--) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
			double predicted_score_error = run_helper.target_val - run_helper.predicted_score;

			double new_score_network_target_max_update;
			if (this->state_iter <= 100000) {
				new_score_network_target_max_update = 0.05;
			} else {
				new_score_network_target_max_update = 0.01;
			}
			vector<double> empty_state_vals;
			// if (this->state_iter <= 300000) {
			// 	this->step_score_networks[a_index]->new_scaled_backprop(run_helper.scale_factor*predicted_score_error,
			// 															run_helper.new_state_errors,
			// 															new_score_network_target_max_update,
			// 															empty_state_vals,
			// 															history->step_ending_new_state_vals_snapshots[a_index],
			// 															history->step_score_network_histories[a_index]);
			// } else {
			// 	this->step_score_networks[a_index]->new_lasso_backprop(run_helper.scale_factor*predicted_score_error,
			// 														   run_helper.new_state_errors,
			// 														   new_score_network_target_max_update,
			// 														   empty_state_vals,
			// 														   history->step_ending_new_state_vals_snapshots[a_index],
			// 														   history->step_score_network_histories[a_index]);
			// }
			this->step_score_networks[a_index]->new_backprop(run_helper.scale_factor*predicted_score_error,
																	   run_helper.new_state_errors,
																	   new_score_network_target_max_update,
																	   empty_state_vals,
																	   history->step_ending_new_state_vals_snapshots[a_index],
																	   history->step_score_network_histories[a_index]);

			run_helper.predicted_score -= run_helper.scale_factor*history->step_score_network_outputs[a_index];

			vector<double> new_state_errors_snapshot = run_helper.new_state_errors;
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				if (history->step_state_network_histories[a_index][s_index] != NULL) {
					double state_network_target_max_update;
					if (this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else {
						state_network_target_max_update = 0.01;
					}
					if (this->state_iter <= 300000) {
						vector<double> empty_state_vals;
						this->step_state_networks[a_index][s_index]->new_scaled_backprop(
							new_state_errors_snapshot[s_index],
							run_helper.new_state_errors,
							state_network_target_max_update,
							history->step_obs_snapshots[a_index],
							empty_state_vals,
							history->step_starting_new_state_vals_snapshots[a_index],
							history->step_state_network_histories[a_index][s_index]);
					} else {
						vector<double> empty_state_vals;
						this->step_state_networks[a_index][s_index]->new_lasso_backprop(
							new_state_errors_snapshot[s_index],
							run_helper.new_state_errors,
							state_network_target_max_update,
							history->step_obs_snapshots[a_index],
							empty_state_vals,
							history->step_starting_new_state_vals_snapshots[a_index],
							history->step_state_network_histories[a_index][s_index]);
					}
				}
			}
			for (int st_index = 0; st_index < (int)history->step_input_sequence_step_indexes[a_index].size(); st_index++) {
				for (int i_index = 0; i_index < (int)history->step_input_state_network_histories[a_index][st_index].size(); i_index++) {
					if (history->step_input_state_network_histories[a_index][st_index][i_index] != NULL) {
						StateNetwork* network = history->step_input_state_network_histories[a_index][st_index][i_index]->network;
						double new_input_network_target_max_update;
						if (this->state_iter <= 100000) {
							new_input_network_target_max_update = 0.05;
						} else {
							new_input_network_target_max_update = 0.01;
						}
						if (this->state_iter <= 300000) {
							vector<double> empty_state_vals;
							network->new_scaled_backprop(run_helper.new_input_errors[history->step_input_sequence_step_indexes[a_index][st_index]][i_index],
														 run_helper.new_state_errors,
														 run_helper.new_input_errors[history->step_input_sequence_step_indexes[a_index][st_index]][i_index],
														 new_input_network_target_max_update,
														 history->step_obs_snapshots[a_index],
														 empty_state_vals,
														 history->step_starting_new_state_vals_snapshots[a_index],
														 history->step_input_vals_snapshots[a_index][st_index][i_index],
														 history->step_input_state_network_histories[a_index][st_index][i_index]);
						} else {
							vector<double> empty_state_vals;
							network->new_lasso_backprop(run_helper.new_input_errors[history->step_input_sequence_step_indexes[a_index][st_index]][i_index],
														run_helper.new_state_errors,
														run_helper.new_input_errors[history->step_input_sequence_step_indexes[a_index][st_index]][i_index],
														new_input_network_target_max_update,
														history->step_obs_snapshots[a_index],
														empty_state_vals,
														history->step_starting_new_state_vals_snapshots[a_index],
														history->step_input_vals_snapshots[a_index][st_index][i_index],
														history->step_input_state_network_histories[a_index][st_index][i_index]);
						}
					}
				}
			}
		} else {
			run_helper.scale_factor *= this->sequence_scale_mods[a_index]->weight;

			double inner_scale_factor_error = 0.0;

			this->sequences[a_index]->backprop(sequence_input_errors[a_index],
											   inner_scale_factor_error,
											   run_helper,
											   history->sequence_histories[a_index]);

			this->sequences[a_index]->backprop_reset(sequence_input_errors[a_index],
													 context,
													 sequence_input_errors);
			run_helper.new_input_errors[a_index] = sequence_input_errors[a_index];

			this->sequence_scale_mods[a_index]->backprop(inner_scale_factor_error, 0.002);

			run_helper.scale_factor /= this->sequence_scale_mods[a_index]->weight;
		}
	}

	this->starting_score_network->new_activate(history->starting_state_vals_snapshot,
											   history->starting_new_state_vals_snapshot);
	double starting_predicted_score = history->existing_predicted_score
		+ run_helper.scale_factor*this->starting_score_network->output->acti_vals[0];
	double starting_predicted_score_error = run_helper.target_val - starting_predicted_score;
	double starting_score_network_target_max_update;
	if (this->state_iter <= 100000) {
		starting_score_network_target_max_update = 0.05;
	} else {
		starting_score_network_target_max_update = 0.01;
	}
	if (this->state_iter <= 300000) {
		this->starting_score_network->new_scaled_backprop(
			run_helper.scale_factor*starting_predicted_score_error,
			run_helper.new_state_errors,
			starting_score_network_target_max_update);
	} else {
		this->starting_score_network->new_lasso_backprop(
			run_helper.scale_factor*starting_predicted_score_error,
			run_helper.new_state_errors,
			starting_score_network_target_max_update);
	}

	this->starting_misguess_network->new_activate(history->starting_state_vals_snapshot,
												  history->starting_new_state_vals_snapshot);
	double starting_misguess_error = run_helper.final_misguess - this->starting_misguess_network->output->acti_vals[0];
	double starting_misguess_network_target_max_update;
	if (this->state_iter <= 100000) {
		starting_misguess_network_target_max_update = 0.05;
	} else {
		starting_misguess_network_target_max_update = 0.01;
	}
	if (this->state_iter <= 300000) {
		this->starting_misguess_network->new_scaled_backprop(
			starting_misguess_error,
			run_helper.new_state_errors,	// don't need to backprop error signals, but definitely not bad to do so
			starting_misguess_network_target_max_update);
	} else {
		this->starting_misguess_network->new_lasso_backprop(
			starting_misguess_error,
			run_helper.new_state_errors,	// don't need to backprop error signals, but definitely not bad to do so
			starting_misguess_network_target_max_update);
	}

	this->existing_misguess_network->activate(history->starting_state_vals_snapshot);

	bool is_branch = false;
	double score_val = run_helper.scale_factor*this->starting_score_network->output->acti_vals[0]
		/ (solution->average_misguess*abs(run_helper.scale_factor));
	if (score_val > 0.1) {
		is_branch = true;
	} else if (score_val > -0.1) {
		double misguess_diff = this->starting_misguess_network->output->acti_vals[0]
			- this->existing_misguess_network->output->acti_vals[0];
		double misguess_val = misguess_diff / (solution->misguess_standard_deviation*abs(run_helper.scale_factor));
		if (misguess_val < -0.1) {
			is_branch = true;
		}
	}
	if (is_branch) {
		this->new_average_score = 0.9999*this->new_average_score + 0.0001*run_helper.target_val;
		this->existing_average_score = 0.9999*this->existing_average_score + 0.0001*history->existing_predicted_score;
		this->new_average_misguess = 0.9999*this->new_average_misguess + 0.0001*run_helper.final_misguess;
		this->existing_average_misguess = 0.9999*this->existing_average_misguess + 0.0001*this->existing_misguess_network->output->acti_vals[0];

		this->branch_weight = 0.9999*this->branch_weight + 0.0001;
	} else {
		this->branch_weight = 0.9999*this->branch_weight + 0.0;
	}

	run_helper.backprop_is_pre_experiment = true;
}
