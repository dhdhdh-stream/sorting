#include "retrain_branch_experiment.h"

#include <cmath>

#include "branch_node.h"
#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void RetrainBranchExperiment::measure_existing_activate(
		bool& is_branch,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	double original_score = this->branch_node->original_score_mod;
	double branch_score = this->branch_node->branch_score_mod;

	for (int s_index = 0; s_index < (int)this->branch_node->decision_state_is_local.size(); s_index++) {
		if (this->branch_node->decision_state_is_local[s_index]) {
			map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->branch_node->decision_state_indexes[s_index]);
			if (it != context.back().local_state_vals.end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					original_score += this->branch_node->decision_original_weights[s_index] * normalized;
					branch_score += this->branch_node->decision_branch_weights[s_index] * normalized;
				} else {
					original_score += this->branch_node->decision_original_weights[s_index] * it->second.val;
					branch_score += this->branch_node->decision_branch_weights[s_index] * it->second.val;
				}
			}
		} else {
			map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->branch_node->decision_state_indexes[s_index]);
			if (it != context.back().input_state_vals.end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					original_score += this->branch_node->decision_original_weights[s_index] * normalized;
					branch_score += this->branch_node->decision_branch_weights[s_index] * normalized;
				} else {
					original_score += this->branch_node->decision_original_weights[s_index] * it->second.val;
					branch_score += this->branch_node->decision_branch_weights[s_index] * it->second.val;
				}
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
	if (abs(branch_score - original_score) > DECISION_MIN_SCORE_IMPACT * this->branch_node->decision_standard_deviation) {
		decision_is_branch = branch_score > original_score;
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

void RetrainBranchExperiment::measure_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (!run_helper.exceeded_limit) {
		if (run_helper.max_depth > solution->max_depth) {
			solution->max_depth = run_helper.max_depth;

			if (solution->max_depth < 50) {
				solution->depth_limit = solution->max_depth + 10;
			} else {
				solution->depth_limit = (int)(1.2*(double)solution->max_depth);
			}
		}
	}

	if ((int)this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / solution->curr_num_datapoints;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_variance = sum_score_variance / solution->curr_num_datapoints;

		this->existing_standard_deviation = sqrt(this->existing_score_variance);

		this->o_target_val_histories.clear();

		this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
	}
}
