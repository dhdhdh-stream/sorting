#include "branch_experiment.h"

#include "abstract_node.h"
#include "action_node.h"
#include "constants.h"
#include "exit_network.h"
#include "layer.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "sequence.h"
#include "state_network.h"

using namespace std;

void BranchExperiment::clean_pre_activate_helper(
		bool on_path,
		double& temp_scale_factor,
		vector<int> temp_scope_context,
		vector<int> temp_node_context,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator state_it = this->state_networks.find(scope_id);
	map<int, vector<ScoreNetwork*>>::iterator score_it = this->score_networks.find(scope_id);

	if (this->state == EXPERIMENT_STATE_SECOND_CLEAN) {
		for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
			set<int>::iterator needed_it = this->scope_additions_needed[s_index].find(scope_id);
			if (needed_it != this->scope_additions_needed[s_index].end()) {
				// if needed, then starting must be on path
				int starting_index;
				if (this->new_state_furthest_layer_needed_in[s_index] == 0) {
					starting_index = 0;
				} else {
					starting_index = run_helper.experiment_context_start_index
						+ this->new_state_furthest_layer_needed_in[s_index]-1;
					// new_state_furthest_layer_needed_in[s_index] < scope_context.size()+2
				}
				for (int c_index = starting_index; c_index < (int)temp_scope_context.size(); c_index++) {
					this->scope_node_additions_needed[s_index].insert({temp_scope_context[c_index], temp_node_context[c_index]});
				}
			}
		}
	}

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->node->id;

				if (state_it != this->state_networks.end()
						&& node_id < (int)state_it->second.size()
						&& (int)state_it->second[node_id].size() != 0) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

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
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				temp_scale_factor *= scope_node->scope_scale_mod->weight;

				temp_scope_context.push_back(scope_id);
				temp_node_context.push_back(scope_node->id);

				if (on_path
						&& i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					clean_pre_activate_helper(false,
											  temp_scale_factor,
											  temp_scope_context,
											  temp_node_context,
											  run_helper,
											  scope_node_history->inner_scope_history);

					temp_scope_context.pop_back();
					temp_node_context.pop_back();

					temp_scale_factor /= scope_node->scope_scale_mod->weight;
				}
			}
		}
	}
}

void BranchExperiment::clean_activate(vector<double>& flat_vals,
									  vector<ForwardContextLayer>& context,
									  RunHelper& run_helper,
									  BranchExperimentHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_CLEAN;

	history->existing_predicted_score = run_helper.predicted_score;

	run_helper.experiment = this;
	if (rand()%5 == 0) {
		run_helper.can_zero = true;
	} else {
		run_helper.can_zero = false;
	}
	run_helper.new_state_vals = vector<double>(NUM_NEW_STATES, 0.0);

	double temp_scale_factor = 1.0;
	vector<int> temp_scope_context;
	vector<int> temp_node_context;
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		clean_pre_activate_helper(true,
								  temp_scale_factor,
								  temp_scope_context,
								  temp_node_context,
								  run_helper,
								  context[c_index].scope_history);

		if (this->state == EXPERIMENT_STATE_SECOND_CLEAN) {
			int context_size_diff = (int)context.size() - (int)this->scope_context.size() - 1;
			for (int cc_index = 0; cc_index < (int)this->corr_calc_scope_depths.size(); cc_index++) {
				if (c_index == (int)context.size()-1 - this->corr_calc_scope_depths[cc_index]) {
					double curr_val = context[context.size()-1 - this->corr_calc_scope_depths[cc_index]].state_vals->at(this->corr_calc_input_indexes[cc_index]);
					this->corr_calc_average_vals[cc_index] = 0.9999*this->corr_calc_average_vals[cc_index] + 0.0001*curr_val;
					double curr_variance = (this->corr_calc_average_vals[cc_index] - curr_val)*(this->corr_calc_average_vals[cc_index] - curr_val);
					this->corr_calc_variances[cc_index] = 0.9999*this->corr_calc_variances[cc_index] + 0.0001*curr_variance;

					for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
						if (this->new_state_furthest_layer_needed_in[s_index] <= c_index - context_size_diff) {
							// c_index - context_size_diff >= 0
							double curr_new_val = run_helper.new_state_vals[s_index];
							this->corr_calc_new_average_vals[cc_index][s_index] = 0.9999*this->corr_calc_new_average_vals[cc_index][s_index] + 0.0001*curr_new_val;
							double curr_new_variance = (this->corr_calc_new_average_vals[cc_index][s_index] - curr_new_val)*(this->corr_calc_new_average_vals[cc_index][s_index] - curr_new_val);
							this->corr_calc_new_variances[cc_index][s_index] = 0.9999*this->corr_calc_new_variances[cc_index][s_index] + 0.0001*curr_new_variance;
							double curr_covariance = (this->corr_calc_average_vals[cc_index] - curr_val)*(this->corr_calc_new_average_vals[cc_index][s_index] - curr_new_val);
							this->corr_calc_covariances[cc_index][s_index] = 0.9999*this->corr_calc_covariances[cc_index][s_index] + 0.0001*curr_covariance;
							this->new_transformations[cc_index][s_index].backprop(curr_val, curr_new_val);
						}
					}
				}
			}
		}
	}

	history->starting_state_vals_snapshot = *(context.back().state_vals);
	history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

	// no need to activate starting_score_network until backprop

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

	run_helper.experiment_context_start_index = (int)context.size() - (int)this->scope_context.size();
	// don't include last layer, which is from ActionNode
	for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		run_helper.experiment_helper_scope_context.push_back(context[c_index].scope_id);
		run_helper.experiment_helper_node_context.push_back(context[c_index].node_id);
	}

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
				} else if (this->step_state_networks[a_index][s_index] == NULL) {
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

			flat_vals.erase(flat_vals.begin());
		} else {
			sequence_input_vals[a_index] = vector<double>(this->sequences[a_index]->input_types.size(), 0.0);
			this->sequences[a_index]->activate_pull(sequence_input_vals[a_index],
													context,
													sequence_input_vals,
													history,
													run_helper);

			run_helper.scale_factor *= this->sequence_scale_mods[a_index]->weight;

			// no longer need to track experiment_step_index

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

	vector<double>* outer_state_vals = context[context.size() - (this->exit_depth+1)].state_vals;
	vector<bool>* outer_states_initialized = &(context[context.size() - (this->exit_depth+1)].states_initialized);

	history->exit_network_histories = vector<ExitNetworkHistory*>(this->exit_networks.size(), NULL);
	for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
		if (outer_states_initialized->at(s_index)) {
			if (this->exit_networks[s_index] != NULL) {
				ExitNetworkHistory* network_history = new ExitNetworkHistory(this->exit_networks[s_index]);
				this->exit_networks[s_index]->new_activate(history->exit_state_vals_snapshot,
														   history->ending_new_state_vals_snapshot,
														   network_history);
				history->exit_network_histories[s_index] = network_history;
				outer_state_vals->at(s_index) += this->exit_networks[s_index]->output->acti_vals[0];

				this->exit_network_impacts[s_index] = 0.9999*this->exit_network_impacts[s_index] + 0.0001*abs(this->exit_networks[s_index]->output->acti_vals[0]);
			}
		}
	}

	run_helper.experiment_on_path = true;
	run_helper.experiment_context_index--;
	// run_helper.experiment_context_index still has to be >0
}

void BranchExperiment::clean_backprop(vector<BackwardContextLayer>& context,
									  RunHelper& run_helper,
									  BranchExperimentHistory* history) {
	vector<double>* outer_state_errors = context[context.size() - (this->exit_depth+1)].state_errors;

	for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
		if (history->exit_network_histories[s_index] != NULL) {
			this->exit_networks[s_index]->new_backprop(
				outer_state_errors->at(s_index),
				run_helper.new_state_errors,
				0.01,
				history->exit_state_vals_snapshot,
				history->ending_new_state_vals_snapshot,
				history->exit_network_histories[s_index]);
		}
	}

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
			vector<double> new_state_errors_snapshot = run_helper.new_state_errors;
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				if (history->step_state_network_histories[a_index][s_index] != NULL) {
					vector<double> empty_state_vals;
					this->step_state_networks[a_index][s_index]->new_backprop(
						new_state_errors_snapshot[s_index],
						run_helper.new_state_errors,
						0.01,
						history->step_obs_snapshots[a_index],
						empty_state_vals,
						history->step_starting_new_state_vals_snapshots[a_index],
						history->step_state_network_histories[a_index][s_index]);
				}
			}
			if (this->state == EXPERIMENT_STATE_FIRST_CLEAN) {
				for (int st_index = 0; st_index < (int)history->step_input_sequence_step_indexes[a_index].size(); st_index++) {
					for (int i_index = 0; i_index < (int)history->step_input_state_network_histories[a_index][st_index].size(); i_index++) {
						if (history->step_input_state_network_histories[a_index][st_index][i_index] != NULL) {
							StateNetwork* network = history->step_input_state_network_histories[a_index][st_index][i_index]->network;
							vector<double> empty_state_vals;
							network->new_lasso_backprop(run_helper.new_input_errors[history->step_input_sequence_step_indexes[a_index][st_index]][i_index],
														run_helper.new_state_errors,
														run_helper.new_input_errors[history->step_input_sequence_step_indexes[a_index][st_index]][i_index],
														0.01,
														history->step_obs_snapshots[a_index],
														empty_state_vals,
														history->step_starting_new_state_vals_snapshots[a_index],
														history->step_input_vals_snapshots[a_index][st_index][i_index],
														history->step_input_state_network_histories[a_index][st_index][i_index]);
						}
					}
				}
			} else {
				for (int st_index = 0; st_index < (int)history->step_input_sequence_step_indexes[a_index].size(); st_index++) {
					for (int i_index = 0; i_index < (int)history->step_input_state_network_histories[a_index][st_index].size(); i_index++) {
						if (history->step_input_state_network_histories[a_index][st_index][i_index] != NULL) {
							StateNetwork* network = history->step_input_state_network_histories[a_index][st_index][i_index]->network;
							vector<double> empty_state_vals;
							network->new_backprop(run_helper.new_input_errors[history->step_input_sequence_step_indexes[a_index][st_index]][i_index],
												  run_helper.new_state_errors,
												  run_helper.new_input_errors[history->step_input_sequence_step_indexes[a_index][st_index]][i_index],
												  0.01,
												  history->step_obs_snapshots[a_index],
												  empty_state_vals,
												  history->step_starting_new_state_vals_snapshots[a_index],
												  history->step_input_vals_snapshots[a_index][st_index][i_index],
												  history->step_input_state_network_histories[a_index][st_index][i_index]);
						}
					}
				}
			}

			double predicted_score_error = run_helper.target_val - run_helper.predicted_score;

			vector<double> empty_state_vals;
			this->step_score_networks[a_index]->new_backprop(run_helper.scale_factor*predicted_score_error,
															 run_helper.new_state_errors,
															 0.01,
															 empty_state_vals,
															 history->step_ending_new_state_vals_snapshots[a_index],
															 history->step_score_network_histories[a_index]);

			run_helper.predicted_score -= run_helper.scale_factor*history->step_score_network_outputs[a_index];
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
	this->starting_score_network->new_backprop(
		run_helper.scale_factor*starting_predicted_score_error,
		run_helper.new_state_errors,
		0.01);

	this->starting_misguess_network->new_activate(history->starting_state_vals_snapshot,
												  history->starting_new_state_vals_snapshot);
	double starting_misguess_error = run_helper.final_misguess - this->starting_misguess_network->output->acti_vals[0];
	this->starting_misguess_network->new_backprop(
		starting_misguess_error,
		run_helper.new_state_errors,
		0.01);

	run_helper.backprop_is_pre_experiment = true;
}
