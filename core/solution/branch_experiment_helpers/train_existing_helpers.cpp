#include "branch_experiment.h"

#include <iostream>

#include "constants.h"
#include "scope.h"
#include "state_network.h"

using namespace std;

void BranchExperiment::train_existing_activate(vector<ContextLayer>& context) {
	for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
			it != context.back().input_state_vals.end(); it++) {
		if (it->first < this->containing_scope_num_input_states) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				(*this->existing_starting_state_vals)(this->state_iter, it->first) = normalized;
			} else {
				(*this->existing_starting_state_vals)(this->state_iter, it->first) = it->second.val;
			}
		}
	}

	Scope* containing_scope = context.back().scope_history->scope;
	int num_input_states = containing_scope->num_input_states;
	for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
			it != context.back().local_state_vals.end(); it++) {
		if (it->first < this->containing_scope_num_local_states) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				(*this->existing_starting_state_vals)(this->state_iter, num_input_states + it->first) = normalized;
			} else {
				(*this->existing_starting_state_vals)(this->state_iter, num_input_states + it->first) = it->second.val;
			}
		}
	}
}

void BranchExperiment::train_existing_backprop(double target_val,
											   BranchExperimentHistory* history) {
	this->existing_average_score += target_val;

	{
		ScopeHistory* parent_scope_history = history->parent_scope_history;
		Scope* parent_scope = parent_scope_history->scope;

		double predicted_score = parent_scope->average_score;

		for (map<int, StateStatus>::iterator it = parent_scope_history->input_state_snapshots.begin();
				it != parent_scope_history->input_state_snapshots.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				predicted_score += parent_scope->input_state_weights[it->first] * normalized;
			} else {
				predicted_score += parent_scope->input_state_weights[it->first] * it->second.val;
			}
		}

		for (map<int, StateStatus>::iterator it = parent_scope_history->local_state_snapshots.begin();
				it != parent_scope_history->local_state_snapshots.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				predicted_score += parent_scope->local_state_weights[it->first] * normalized;
			} else {
				predicted_score += parent_scope->local_state_weights[it->first] * it->second.val;
			}
		}

		double curr_misguess = (target_val - predicted_score) * (target_val - predicted_score);
		this->existing_average_misguess += curr_misguess;
	}

	this->existing_target_vals[this->state_iter] = target_val;

	this->state_iter++;
	if (this->state_iter >= NUM_DATAPOINTS) {
		this->existing_average_score /= NUM_DATAPOINTS;
		this->existing_average_misguess /= NUM_DATAPOINTS;

		Eigen::VectorXd v_existing_target_vals(NUM_DATAPOINTS);
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			v_existing_target_vals(d_index) = this->existing_target_vals[d_index] - this->existing_average_score;
		}

		Eigen::VectorXd result = (*this->existing_starting_state_vals).fullPivHouseholderQr().solve(v_existing_target_vals);

		this->existing_starting_input_state_weights = vector<double>(this->containing_scope_num_input_states);
		for (int s_index = 0; s_index < this->containing_scope_num_input_states; s_index++) {
			this->existing_starting_input_state_weights[s_index] = result(s_index);
		}
		this->existing_starting_local_state_weights = vector<double>(this->containing_scope_num_local_states);
		for (int s_index = 0; s_index < this->containing_scope_num_local_states; s_index++) {
			this->existing_starting_local_state_weights[s_index] = result(this->containing_scope_num_input_states + s_index);
		}

		delete this->existing_starting_state_vals;
		this->existing_starting_state_vals = NULL;
		this->existing_target_vals.clear();

		this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
