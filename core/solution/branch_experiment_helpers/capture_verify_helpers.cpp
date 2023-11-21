#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "potential_scope_node.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void BranchExperiment::capture_verify_activate(
		AbstractNode*& curr_node,
		Problem& problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	if (this->state_iter == 0) {
		cout << "input_state_vals" << endl;
		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			cout << "c_index: " << c_index << endl;
			for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
					it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
				map<int, double>::iterator original_weight_it = this->existing_input_state_weights[c_index].find(it->first);
				if (original_weight_it != this->existing_input_state_weights[c_index].end()) {
					cout << it->second.val << endl;
				}
			}
		}
		cout << "local_state_vals" << endl;
		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			cout << "c_index: " << c_index << endl;
			for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
					it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
				map<int, double>::iterator original_weight_it = this->existing_local_state_weights[c_index].find(it->first);
				if (original_weight_it != this->existing_local_state_weights[c_index].end()) {
					cout << it->second.val << endl;
				}
			}
		}
		cout << "temp_state_vals" << endl;
		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			cout << "c_index: " << c_index << endl;
			for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
					it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
				map<State*, double>::iterator original_weight_it = this->existing_temp_state_weights[c_index].find(it->first);
				if (original_weight_it != this->existing_temp_state_weights[c_index].end()) {
					cout << it->first->id << " " << it->second.val << endl;
				}
			}
		}
	}

	double original_predicted_score = this->existing_average_score;
	double branch_predicted_score = this->new_average_score;

	vector<double> factors;

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<int, double>::iterator original_weight_it = this->existing_input_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_input_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<int, double>::iterator branch_weight_it = this->new_input_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_input_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;

				if (original_weight_it != this->existing_input_state_weights[c_index].end()) {
					factors.push_back(normalized);
				}
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;

				if (original_weight_it != this->existing_input_state_weights[c_index].end()) {
					factors.push_back(it->second.val);
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<int, double>::iterator original_weight_it = this->existing_local_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_local_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<int, double>::iterator branch_weight_it = this->new_local_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_local_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;

				if (original_weight_it != this->existing_local_state_weights[c_index].end()) {
					factors.push_back(normalized);
				}
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;

				if (original_weight_it != this->existing_local_state_weights[c_index].end()) {
					factors.push_back(it->second.val);
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		/**
		 * - insert val to match behavior after finalize
		 */
		for (map<State*, double>::iterator weight_it = this->existing_temp_state_weights[c_index].begin();
				weight_it != this->existing_temp_state_weights[c_index].end(); weight_it++) {
			map<State*, StateStatus>::iterator val_it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.find(weight_it->first);
			if (val_it == context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end()) {
				context[context.size() - this->scope_context.size() + c_index].temp_state_vals[weight_it->first] = StateStatus();
			}
		}

		for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<State*, double>::iterator original_weight_it = this->existing_temp_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_temp_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<State*, double>::iterator branch_weight_it = this->new_temp_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_temp_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;

				if (original_weight_it != this->existing_temp_state_weights[c_index].end()) {
					factors.push_back(normalized);
				}
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;

				if (original_weight_it != this->existing_temp_state_weights[c_index].end()) {
					factors.push_back(it->second.val);
				}
			}
		}
	}

	Problem curr_problem = problem;
	curr_problem.current_world = curr_problem.initial_world;
	curr_problem.current_pointer = 0;
	this->verify_problems[this->state_iter] = curr_problem;

	this->verify_original_scores.push_back(original_predicted_score);
	this->verify_branch_scores.push_back(branch_predicted_score);
	this->verify_factors.push_back(factors);

	if (branch_predicted_score > original_predicted_score) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
				this->best_actions[s_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					action_node_history);
				delete action_node_history;
			} else {
				this->best_potential_scopes[s_index]->capture_verify_activate(
					problem,
					context,
					run_helper);
			}
		}

		if (this->best_exit_depth == 0) {
			curr_node = this->best_exit_node;
		} else {
			curr_node = NULL;

			exit_depth = this->best_exit_depth-1;
			exit_node = this->best_exit_node;
		}
	}
}

void BranchExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
				this->best_potential_scopes[s_index]->scope_node_placeholder->verify_key = this;
			}
		}
		solution->verify_key = this;
		solution->verify_problems = this->verify_problems;

		this->state = BRANCH_EXPERIMENT_STATE_SUCCESS;
	}
}
