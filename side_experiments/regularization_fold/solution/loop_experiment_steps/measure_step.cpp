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

void LoopExperiment::measure_pre_activate_helper(
		double& temp_scale_factor,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator state_it = this->state_networks.find(scope_id);
	map<int, vector<ScoreNetwork*>>::iterator score_it = this->score_networks.find(scope_id);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->node->id;

				if (state_it != this->state_networks.end()
						&& node_id < (int)state_it->second.size()
						&& state_it->second[node_id].size() != 0) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

					action_node_history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

					for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
						StateNetwork* network = state_it->second[node_id][s_index];
						network->new_activate(action_node_history->obs_snapshot,
											  action_node_history->starting_state_vals_snapshot,
											  action_node_history->starting_new_state_vals_snapshot);
						run_helper.new_state_vals[s_index] += network->output->acti_vals[0];
					}

					ScoreNetwork* score_network = score_it->second[node_id];
					score_network->new_activate(action_node_history->ending_state_vals_snapshot,
												run_helper.new_state_vals);

					run_helper.predicted_score += temp_scale_factor*score_network->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				temp_scale_factor *= scope_node->scope_scale_mod->weight;

				measure_pre_activate_helper(temp_scale_factor,
											run_helper,
											scope_node_history->inner_scope_history);

				temp_scale_factor /= scope_node->scope_scale_mod->weight;
			}
		}
	}
}

void LoopExperiment::measure_activate(vector<double>& flat_vals,
									  vector<ForwardContextLayer>& context,
									  RunHelper& run_helper) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT;

	this->existing_average_score += run_helper.predicted_score;
	this->existing_misguess_network->activate(*(context.back().state_vals));
	this->existing_average_misguess += this->existing_misguess_network->output->acti_vals[0];

	run_helper.experiment = this;
	run_helper.can_zero = false;
	run_helper.new_state_vals = vector<double>(NUM_NEW_STATES, 0.0);

	double temp_scale_factor = 1.0;
	measure_pre_activate_helper(temp_scale_factor,
								run_helper,
								context[0].scope_history);

	vector<double> input_vals(this->sequence->input_types.size(), 0.0);
	vector<vector<double>> empty_previous_vals;
	this->sequence->activate_pull(input_vals,
								  context,
								  empty_previous_vals,
								  NULL,
								  run_helper);

	int iter_index = 0;
	while (true) {
		if (iter_index > 7) {
			// cap at 8 iters for experiment
			break;
		}

		this->continue_score_network->new_activate(input_vals,
												   run_helper.new_state_vals);
		double continue_score = run_helper.scale_factor*this->continue_score_network->output->acti_vals[0];

		this->halt_score_network->new_activate(input_vals,
											   run_helper.new_state_vals);
		double halt_score = run_helper.scale_factor*this->halt_score_network->output->acti_vals[0];

		double score_diff = continue_score - halt_score;
		double score_val = score_diff / (solution->average_misguess*abs(run_helper.scale_factor));
		if (score_val > 0.1) {
			// continue
		} else if (score_val < -0.1) {
			break;
		} else {
			this->continue_misguess_network->new_activate(input_vals,
														  run_helper.new_state_vals);

			this->halt_misguess_network->new_activate(input_vals,
													  run_helper.new_state_vals);

			double misguess_diff = this->continue_misguess_network->output->acti_vals[0]
				- this->halt_misguess_network->output->acti_vals[0];
			double misguess_val = misguess_diff / (solution->misguess_standard_deviation*abs(run_helper.scale_factor));
			if (misguess_val < -0.1) {
				// continue
			} else if (misguess_val > 0.1) {
				break;
			} else {
				// halt if no strong signal either way
				break;
			}
		}

		run_helper.scale_factor *= this->scale_mod->weight;

		SequenceHistory* sequence_history = new SequenceHistory(this->sequence);
		this->sequence->activate(input_vals,
								 flat_vals,
								 run_helper,
								 sequence_history);
		delete sequence_history;

		run_helper.scale_factor /= this->scale_mod->weight;

		iter_index++;
	}

	this->sequence->activate_reset(input_vals,
								   context,
								   empty_previous_vals);

	vector<vector<double>> exit_state_vals_snapshot(1);
	exit_state_vals_snapshot[0] = *(context.back().state_vals);

	vector<double>* outer_state_vals = context.back().state_vals;
	vector<bool>* outer_states_initialized = &(context.back().states_initialized);

	for (int s_index = 0; s_index < (int)this->exit_networks.size(); s_index++) {
		if (outer_states_initialized->at(s_index)) {
			this->exit_networks[s_index]->new_activate(exit_state_vals_snapshot,
													   run_helper.new_state_vals);
			outer_state_vals->at(s_index) += this->exit_networks[s_index]->output->acti_vals[0];
		}
	}
}
