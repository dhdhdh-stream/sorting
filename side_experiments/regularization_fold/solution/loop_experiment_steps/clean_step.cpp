#include "loop_experiment.h"

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

void LoopExperiment::clean_pre_activate_helper(
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

void LoopExperiment::clean_activate(vector<double>& flat_vals,
									vector<ForwardContextLayer>& context,
									RunHelper& run_helper,
									LoopExperimentHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_CLEAN;

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
					double curr_val = context[(int)context.size()-1 - this->corr_calc_scope_depths[cc_index]].state_vals->at(this->corr_calc_input_indexes[cc_index]);
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

	int loop_iters = rand()%7;

	run_helper.experiment_on_path = false;
	run_helper.experiment_context_index = (int)this->scope_context.size()+1;

	run_helper.experiment_context_start_index = (int)context.size() - (int)this->scope_context.size();
	// don't include last layer, which is from ActionNode
	for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		run_helper.experiment_helper_scope_context.push_back(context[c_index].scope_id);
		run_helper.experiment_helper_node_context.push_back(context[c_index].node_id);
	}

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
	this->halt_score_network->new_activate(input_vals,
										   run_helper.new_state_vals,
										   halt_score_network_history);
	history->halt_score_network_history = halt_score_network_history;
	history->halt_score_network_output = this->halt_score_network->output->acti_vals[0];
	history->halt_score_snapshots.push_back(
		run_helper.predicted_score + run_helper.scale_factor*this->halt_score_network->output->acti_vals[0]);

	ScoreNetworkHistory* halt_misguess_network_history = new ScoreNetworkHistory(this->halt_misguess_network);
	this->halt_misguess_network->new_activate(input_vals,
											  run_helper.new_state_vals,
											  halt_misguess_network_history);
	history->halt_misguess_network_history = halt_misguess_network_history;
	history->halt_misguess_network_output = this->halt_misguess_network->output->acti_vals[0];
	history->halt_misguess_snapshots.push_back(this->halt_misguess_network->output->acti_vals[0]);

	this->sequence->activate_reset(input_vals,
								   context,
								   empty_previous_vals);

	history->exit_state_vals_snapshot = vector<vector<double>>(1);
	history->exit_state_vals_snapshot[0] = *(context.back().state_vals);
	history->ending_new_state_vals_snapshot = run_helper.new_state_vals;

	vector<double>* outer_state_vals = context.back().state_vals;
	vector<bool>* outer_states_initialized = &(context.back().states_initialized);

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

void LoopExperiment::clean_backprop(vector<BackwardContextLayer>& context,
									RunHelper& run_helper,
									LoopExperimentHistory* history) {
	vector<double>* outer_state_errors = context.back().state_errors;

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

	vector<double> input_errors(this->sequence->input_types.size(), 0.0);
	vector<vector<double>> empty_previous_errors;
	this->sequence->backprop_pull(input_errors,
								  context,
								  empty_previous_errors);

	double halt_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->halt_score_network_output;
	double halt_predicted_score_error = run_helper.target_val - halt_predicted_score;
	this->halt_score_network->new_backprop(
		run_helper.scale_factor*halt_predicted_score_error,
		run_helper.new_state_errors,
		0.01,
		history->ending_input_vals_snapshot,
		history->ending_new_state_vals_snapshot,
		history->halt_score_network_history);

	double halt_misguess_error = run_helper.final_misguess - history->halt_misguess_network_output;
	this->halt_misguess_network->new_backprop(
		halt_misguess_error,
		run_helper.new_state_errors,
		0.01,
		history->ending_input_vals_snapshot,
		history->ending_new_state_vals_snapshot,
		history->halt_misguess_network_history);

	for (int i_index = (int)history->sequence_histories.size()-1; i_index >= 0; i_index--) {
		run_helper.scale_factor *= this->scale_mod->weight;

		double inner_scale_factor_error = 0.0;

		this->sequence->backprop(input_errors,
								 inner_scale_factor_error,
								 run_helper,
								 history->sequence_histories[i_index]);

		this->scale_mod->backprop(inner_scale_factor_error, 0.002);

		run_helper.scale_factor /= this->scale_mod->weight;

		double best_halt_score = history->halt_score_snapshots.back();
		double best_halt_misguess = history->halt_misguess_snapshots.back();
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
		this->continue_score_network->new_backprop(
			run_helper.scale_factor*continue_predicted_score_error,
			run_helper.new_state_errors,
			0.01,
			history->iter_input_vals_snapshots[i_index],
			history->iter_new_state_vals_snapshots[i_index],
			history->continue_score_network_histories[i_index]);

		double continue_misguess_error = best_halt_misguess - history->continue_misguess_network_outputs[i_index];
		this->continue_misguess_network->new_backprop(
			continue_misguess_error,
			run_helper.new_state_errors,
			0.01,
			history->iter_input_vals_snapshots[i_index],
			history->iter_new_state_vals_snapshots[i_index],
			history->continue_misguess_network_histories[i_index]);
	}

	this->sequence->backprop_reset(input_errors,
								   context,
								   empty_previous_errors);
	run_helper.new_input_errors.push_back(input_errors);

	run_helper.backprop_is_pre_experiment = true;
}
