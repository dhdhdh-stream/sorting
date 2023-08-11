#include "loop_experiment.h"

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
#include "globals.h"
#include "utilities.h"

using namespace std;

void LoopExperiment::wrapup_pre_activate_helper(
		double& temp_scale_factor,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<ScoreNetwork*>>::iterator score_it = this->score_networks.find(scope_id);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->node->id;

				if (score_it != this->score_networks.end()
						&& node_id < (int)score_it->second.size()
						&& score_it->second[node_id] != NULL) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

					ScoreNetwork* score_network = score_it->second[node_id];
					score_network->activate(action_node_history->ending_state_vals_snapshot);
					action_node_history->new_score_network_output = score_network->output->acti_vals[0];

					double temp_score_scale = (50000.0-this->state_iter)/50000.0;
					run_helper.predicted_score += temp_scale_factor*temp_score_scale*score_network->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
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

void LoopExperiment::wrapup_activate(vector<double>& flat_vals,
									 vector<ForwardContextLayer>& context,
									 RunHelper& run_helper,
									 LoopExperimentHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_WRAPUP;

	if (this->state_iter < 50000) {
		double temp_scale_factor = 1.0;
		wrapup_pre_activate_helper(temp_scale_factor,
								   run_helper,
								   context[0].scope_history);
	}

	vector<double> input_vals(this->new_num_states, 0.0);
	for (int i_index = 0; i_index < (int)this->last_layer_indexes.size(); i_index++) {
		input_vals[this->last_layer_target_indexes[i_index]] = context.back().state_vals->at(this->last_layer_indexes[i_index]);
	}
	// don't need to update context and call activate_pull

	int target_iter;
	if (run_helper.can_random_iter && rand()%10 == 0) {
		target_iter = rand()%7;
	} else {
		target_iter = -1;
	}

	int iter_index = 0;
	while (true) {
		ScoreNetworkHistory* halt_score_network_history = new ScoreNetworkHistory(this->halt_score_network);
		this->halt_score_network->activate(input_vals,
										   halt_score_network_history);
		double halt_score = run_helper.scale_factor*this->halt_score_network->output->acti_vals[0];

		history->halt_score_snapshots.push_back(
			run_helper.predicted_score + run_helper.scale_factor*this->halt_score_network->output->acti_vals[0]);

		ScoreNetworkHistory* halt_misguess_network_history = new ScoreNetworkHistory(this->halt_misguess_network);
		this->halt_misguess_network->activate(input_vals,
											  halt_misguess_network_history);

		history->halt_misguess_snapshots.push_back(this->halt_misguess_network->output->acti_vals[0]);

		if (iter_index == target_iter) {
			history->ending_input_vals_snapshot = input_vals;

			history->halt_score_network_history = halt_score_network_history;
			history->halt_score_network_output = this->halt_score_network->output->acti_vals[0];

			history->halt_misguess_network_history = halt_misguess_network_history;
			history->halt_misguess_network_output = this->halt_misguess_network->output->acti_vals[0];

			break;
		}

		if (iter_index > 7) {
			// cap at 8 iters for experiment

			history->ending_input_vals_snapshot = input_vals;

			history->halt_score_network_history = halt_score_network_history;
			history->halt_score_network_output = this->halt_score_network->output->acti_vals[0];

			history->halt_misguess_network_history = halt_misguess_network_history;
			history->halt_misguess_network_output = this->halt_misguess_network->output->acti_vals[0];

			break;
		}

		ScoreNetworkHistory* continue_score_network_history = new ScoreNetworkHistory(this->continue_score_network);
		this->continue_score_network->activate(input_vals,
											   continue_score_network_history);
		double continue_score = run_helper.scale_factor*this->continue_score_network->output->acti_vals[0];

		ScoreNetworkHistory* continue_misguess_network_history = new ScoreNetworkHistory(this->continue_misguess_network);
		this->continue_misguess_network->activate(input_vals,
												  continue_misguess_network_history);

		if (target_iter != -1) {
			delete halt_score_network_history;
			delete halt_misguess_network_history;

			history->iter_input_vals_snapshots.push_back(input_vals);

			history->continue_score_network_histories.push_back(continue_score_network_history);
			history->continue_score_network_outputs.push_back(this->continue_score_network->output->acti_vals[0]);

			history->continue_misguess_network_histories.push_back(continue_misguess_network_history);
			history->continue_misguess_network_outputs.push_back(this->continue_misguess_network->output->acti_vals[0]);

			// continue
		} else {
			double score_diff = continue_score - halt_score;
			double score_val = score_diff / (solution->average_misguess*abs(run_helper.scale_factor));
			if (score_val > 0.1) {
				delete halt_score_network_history;
				delete halt_misguess_network_history;

				history->iter_input_vals_snapshots.push_back(input_vals);

				history->continue_score_network_histories.push_back(continue_score_network_history);
				history->continue_score_network_outputs.push_back(this->continue_score_network->output->acti_vals[0]);

				history->continue_misguess_network_histories.push_back(continue_misguess_network_history);
				history->continue_misguess_network_outputs.push_back(this->continue_misguess_network->output->acti_vals[0]);

				// continue
			} else if (score_val < -0.1) {
				delete continue_score_network_history;
				delete continue_misguess_network_history;

				history->ending_input_vals_snapshot = input_vals;

				history->halt_score_network_history = halt_score_network_history;
				history->halt_score_network_output = this->halt_score_network->output->acti_vals[0];

				history->halt_misguess_network_history = halt_misguess_network_history;
				history->halt_misguess_network_output = this->halt_misguess_network->output->acti_vals[0];

				break;
			} else {
				double misguess_diff = this->continue_misguess_network->output->acti_vals[0]
					- this->halt_misguess_network->output->acti_vals[0];
				double misguess_val = misguess_diff / (solution->misguess_standard_deviation*abs(run_helper.scale_factor));
				if (misguess_val < -0.1) {
					delete halt_score_network_history;
					delete halt_misguess_network_history;

					history->iter_input_vals_snapshots.push_back(input_vals);

					history->continue_score_network_histories.push_back(continue_score_network_history);
					history->continue_score_network_outputs.push_back(this->continue_score_network->output->acti_vals[0]);

					history->continue_misguess_network_histories.push_back(continue_misguess_network_history);
					history->continue_misguess_network_outputs.push_back(this->continue_misguess_network->output->acti_vals[0]);

					// continue
				} else if (misguess_val > 0.1) {
					delete continue_score_network_history;
					delete continue_misguess_network_history;

					history->ending_input_vals_snapshot = input_vals;

					history->halt_score_network_history = halt_score_network_history;
					history->halt_score_network_output = this->halt_score_network->output->acti_vals[0];

					history->halt_misguess_network_history = halt_misguess_network_history;
					history->halt_misguess_network_output = this->halt_misguess_network->output->acti_vals[0];

					break;
				} else {
					// halt if no strong signal either way

					delete continue_score_network_history;
					delete continue_misguess_network_history;

					history->ending_input_vals_snapshot = input_vals;

					history->halt_score_network_history = halt_score_network_history;
					history->halt_score_network_output = this->halt_score_network->output->acti_vals[0];

					history->halt_misguess_network_history = halt_misguess_network_history;
					history->halt_misguess_network_output = this->halt_misguess_network->output->acti_vals[0];

					break;
				}
			}
		}

		run_helper.scale_factor *= this->scale_mod->weight;

		SequenceHistory* sequence_history = new SequenceHistory(this->sequence);
		history->sequence_histories.push_back(sequence_history);
		this->sequence->activate(input_vals,
								 flat_vals,
								 run_helper,
								 sequence_history);

		run_helper.scale_factor /= this->scale_mod->weight;

		iter_index++;
	}

	for (int i_index = 0; i_index < (int)this->last_layer_indexes.size(); i_index++) {
		context.back().state_vals->at(this->last_layer_indexes[i_index]) = input_vals[this->last_layer_target_indexes[i_index]];
	}

	history->exit_state_vals_snapshot = vector<vector<double>>(1);
	history->exit_state_vals_snapshot[0] = *(context.back().state_vals);

	for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
		if (context.back().states_initialized[s_index]) {
			if (this->exit_networks[s_index] != NULL) {
				this->exit_networks[s_index]->activate(history->exit_state_vals_snapshot);
				context.back().state_vals->at(s_index) += this->exit_networks[s_index]->output->acti_vals[0];
			}
		}
	}
}

void LoopExperiment::wrapup_backprop(vector<BackwardContextLayer>& context,
									 double& scale_factor_error,
									 RunHelper& run_helper,
									 LoopExperimentHistory* history) {
	double halt_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->halt_score_network_output;
	double halt_predicted_score_error = run_helper.target_val - halt_predicted_score;
	this->halt_score_network->backprop_weights_with_no_error_signal(
		run_helper.scale_factor*halt_predicted_score_error,
		0.002,
		history->ending_input_vals_snapshot,
		history->halt_score_network_history);

	double halt_misguess_error = run_helper.final_misguess - history->halt_misguess_network_output;
	this->halt_misguess_network->backprop_weights_with_no_error_signal(
		halt_misguess_error,
		0.002,
		history->ending_input_vals_snapshot,
		history->halt_misguess_network_history);

	for (int i_index = (int)history->sequence_histories.size()-1; i_index >= 0; i_index--) {
		run_helper.scale_factor *= this->scale_mod->weight;

		double inner_scale_factor_error = 0.0;

		vector<double> empty_input_errors;
		this->sequence->backprop(empty_input_errors,
								 inner_scale_factor_error,
								 run_helper,
								 history->sequence_histories[i_index]);

		this->scale_mod->backprop(inner_scale_factor_error, 0.0002);

		scale_factor_error += this->scale_mod->weight*inner_scale_factor_error;

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
		this->continue_score_network->backprop_weights_with_no_error_signal(
			run_helper.scale_factor*continue_predicted_score_error,
			0.002,
			history->iter_input_vals_snapshots[i_index],
			history->continue_score_network_histories[i_index]);

		double continue_misguess_error = best_halt_misguess - history->continue_misguess_network_outputs[i_index];
		this->continue_misguess_network->backprop_weights_with_no_error_signal(
			continue_misguess_error,
			0.002,
			history->iter_input_vals_snapshots[i_index],
			history->continue_misguess_network_histories[i_index]);
	}

	// no need to set run_helper.backprop_is_pre_experiment
}
