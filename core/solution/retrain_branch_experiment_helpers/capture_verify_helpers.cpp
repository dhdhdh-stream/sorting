#if defined(MDEBUG) && MDEBUG

#include "retrain_branch_experiment.h"

#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "state_network.h"
#include "globals.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void RetrainBranchExperiment::capture_verify_activate(
		bool& is_branch,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	double original_predicted_score = this->original_average_score;
	double branch_predicted_score = this->branch_average_score;

	vector<double> factors;

	for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->branch_node->branch_scope_context.size() + c_index].input_state_vals.begin();
				it != context[context.size() - this->branch_node->branch_scope_context.size() + c_index].input_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<int, double>::iterator original_weight_it = this->original_input_state_weights[c_index].find(it->first);
			if (original_weight_it != this->original_input_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<int, double>::iterator branch_weight_it = this->branch_input_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->branch_input_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;

				if (original_weight_it != this->original_input_state_weights[c_index].end()) {
					factors.push_back(normalized);
				}
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;

				if (original_weight_it != this->original_input_state_weights[c_index].end()) {
					if (it->second.val != 0.0) {
						factors.push_back(it->second.val);
					}
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->branch_node->branch_scope_context.size() + c_index].local_state_vals.begin();
				it != context[context.size() - this->branch_node->branch_scope_context.size() + c_index].local_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<int, double>::iterator original_weight_it = this->original_local_state_weights[c_index].find(it->first);
			if (original_weight_it != this->original_local_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<int, double>::iterator branch_weight_it = this->branch_local_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->branch_local_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;

				if (original_weight_it != this->original_local_state_weights[c_index].end()) {
					factors.push_back(normalized);
				}
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;

				if (original_weight_it != this->original_local_state_weights[c_index].end()) {
					if (it->second.val != 0.0) {
						factors.push_back(it->second.val);
					}
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
		for (map<State*, StateStatus>::iterator it = context[context.size() - this->branch_node->branch_scope_context.size() + c_index].temp_state_vals.begin();
				it != context[context.size() - this->branch_node->branch_scope_context.size() + c_index].temp_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<State*, double>::iterator original_weight_it = this->original_temp_state_weights[c_index].find(it->first);
			if (original_weight_it != this->original_temp_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<State*, double>::iterator branch_weight_it = this->branch_temp_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->branch_temp_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;

				if (original_weight_it != this->original_temp_state_weights[c_index].end()) {
					factors.push_back(normalized);
				}
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;

				if (original_weight_it != this->original_temp_state_weights[c_index].end()) {
					factors.push_back(it->second.val);
				}
			}
		}
	}

	this->verify_original_scores.push_back(original_predicted_score);
	this->verify_branch_scores.push_back(branch_predicted_score);
	this->verify_factors.push_back(factors);

	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	if (decision_is_branch) {
		is_branch = true;
	} else {
		is_branch = false;
	}
}

void RetrainBranchExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		solution->verify_key = this;
		solution->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		solution->verify_seeds = this->verify_seeds;

		finalize();
	}
}

#endif /* MDEBUG */