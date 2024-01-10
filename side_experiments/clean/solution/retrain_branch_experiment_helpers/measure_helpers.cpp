#include "retrain_branch_experiment.h"

#include <algorithm>

#include "branch_node.h"
#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void RetrainBranchExperiment::measure_activate(bool& is_branch,
											   vector<ContextLayer>& context,
											   RunHelper& run_helper) {
	double original_predicted_score = this->original_average_score;
	double branch_predicted_score = this->branch_average_score;

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

			FullNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
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

			FullNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
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

			FullNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
			}
		}
	}

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch;
	if (abs(branch_predicted_score - original_predicted_score) > DECISION_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
		decision_is_branch = branch_predicted_score > original_predicted_score;
	} else {
		uniform_int_distribution<int> distribution(0, 1);
		decision_is_branch = distribution(generator);
	}
	#endif /* MDEBUG */

	if (decision_is_branch) {
		is_branch = true;
	} else {
		is_branch = false;
	}
}

void RetrainBranchExperiment::measure_backprop(double target_val) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state_iter >= solution->curr_num_datapoints) {
		this->combined_score /= solution->curr_num_datapoints;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double combined_improvement = this->combined_score - this->existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (score_standard_deviation / sqrt(solution->curr_num_datapoints));

		if (combined_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			this->combined_score = 0.0;

			this->o_target_val_histories.reserve(solution->curr_num_datapoints);

			this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_EXISTING;
			this->state_iter = 0;
		} else {
			this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_FAIL;
		}
	}
}
