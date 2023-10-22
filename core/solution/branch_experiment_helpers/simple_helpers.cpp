#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void BranchExperiment::simple_activate(int& curr_node_id,
									   Problem& problem,
									   vector<ContextLayer>& context,
									   int& exit_depth,
									   int& exit_node_id,
									   RunHelper& run_helper) {
	double original_score = this->existing_average_score;
	double branch_score = this->new_average_score;

	for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
			it != context.back().input_state_vals.end(); it++) {
		if (it->first < this->containing_scope_num_input_states) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_score += this->existing_starting_input_state_weights[it->first] * normalized;
				branch_score += this->new_starting_input_state_weights[it->first] * normalized;
			} else {
				original_score += this->existing_starting_input_state_weights[it->first] * it->second.val;
				branch_score += this->new_starting_input_state_weights[it->first] * it->second.val;
			}
		}
	}

	for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
			it != context.back().local_state_vals.end(); it++) {
		if (it->first < this->containing_scope_num_local_states) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_score += this->existing_starting_local_state_weights[it->first] * normalized;
				branch_score += this->new_starting_local_state_weights[it->first] * normalized;
			} else {
				original_score += this->existing_starting_local_state_weights[it->first] * it->second.val;
				branch_score += this->new_starting_local_state_weights[it->first] * it->second.val;
			}
		}
	}

	for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].experiment_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].experiment_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		double normalized = (it->second.val - last_network->ending_mean)
			/ last_network->ending_standard_deviation;
		branch_score += this->new_starting_experiment_state_weights[it->first] * normalized;
	}

	if (branch_score > original_score) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			// leave context.back().node_id as -1

			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				this->best_actions[s_index]->branch_experiment_simple_activate(
					problem);
			} else {
				SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
				this->best_sequences[s_index]->activate(problem,
														context,
														run_helper,
														sequence_history);
				delete sequence_history;
			}
		}

		if (this->best_exit_depth == 0) {
			curr_node_id = this->best_exit_node_id;
		} else {
			exit_depth = this->best_exit_depth-1;
			exit_node_id = this->best_exit_node_id;
		}
	}
}

void BranchExperiment::simple_pass_through_activate(int& curr_node_id,
													Problem& problem,
													vector<ContextLayer>& context,
													int& exit_depth,
													int& exit_node_id,
													RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		// leave context.back().node_id as -1

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->branch_experiment_simple_activate(
				problem);
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
			delete sequence_history;
		}
	}

	if (this->best_exit_depth == 0) {
		curr_node_id = this->best_exit_node_id;
	} else {
		exit_depth = this->best_exit_depth-1;
		exit_node_id = this->best_exit_node_id;
	}
}
