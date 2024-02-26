#include "seed_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"
#include "seed_experiment_filter.h"
#include "solution.h"

using namespace std;

void SeedExperiment::verify_filter_backprop(double target_val,
											SeedExperimentOverallHistory* history) {
	/**
	 * - check history->has_target to make sure filter is reached
	 */
	if (history->has_target) {
		this->curr_filter_score += target_val;

		this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

		this->sub_state_iter++;
		if (this->state == SEED_EXPERIMENT_STATE_VERIFY_1ST_FILTER
				&& this->sub_state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
			this->curr_filter_score /= (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double score_diff = this->curr_filter_score - this->existing_average_score;
			double t_score = score_diff
				/ (this->existing_score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));

			if (t_score > -0.2) {
			#endif /* MDEBUG */
				this->curr_filter_score = 0.0;

				cout << "SEED_EXPERIMENT_STATE_VERIFY_2ND_FILTER" << endl;
				this->state = SEED_EXPERIMENT_STATE_VERIFY_2ND_FILTER;
				/**
				 * - leave this->state_iter unchanged
				 */
				this->sub_state_iter = 0;
			} else {
				AbstractNode* filter_node = this->curr_filter->node_context.back();
				if (filter_node->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)filter_node;
					int experiment_index;
					for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
						if (action_node->experiments[e_index] == this->curr_filter) {
							experiment_index = e_index;
						}
					}
					action_node->experiments.erase(action_node->experiments.begin() + experiment_index);
				} else if (filter_node->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)filter_node;
					int experiment_index;
					for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
						if (scope_node->experiments[e_index] == this->curr_filter) {
							experiment_index = e_index;
						}
					}
					scope_node->experiments.erase(scope_node->experiments.begin() + experiment_index);
				} else {
					BranchNode* branch_node = (BranchNode*)filter_node;
					int experiment_index;
					for (int e_index = 0; e_index < (int)branch_node->experiments.size(); e_index++) {
						if (branch_node->experiments[e_index] == this->curr_filter) {
							experiment_index = e_index;
						}
					}
					branch_node->experiments.erase(branch_node->experiments.begin() + experiment_index);
					branch_node->experiment_types.erase(branch_node->experiment_types.begin() + experiment_index);
				}
				delete this->curr_filter;
				this->curr_filter = NULL;

				this->state_iter++;
				if (this->state_iter >= FIND_FILTER_ITER_LIMIT) {
					cout << "EXPERIMENT_RESULT_FAIL" << endl;
					this->result = EXPERIMENT_RESULT_FAIL;
				} else {
					cout << "SEED_EXPERIMENT_STATE_FIND_FILTER" << endl;
					this->state = SEED_EXPERIMENT_STATE_FIND_FILTER;
					create_filter();
					this->sub_state_iter = 0;
				}
			}
		} else if (this->sub_state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
			this->curr_filter_score /= (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double score_diff = this->curr_filter_score - this->existing_average_score;
			double t_score = score_diff
				/ (this->existing_score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));

			if (t_score > -0.2) {
			#endif /* MDEBUG */
				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_is_higher_histories.reserve(solution->curr_num_datapoints);

				cout << "SEED_EXPERIMENT_STATE_TRAIN_FILTER" << endl;
				this->state = SEED_EXPERIMENT_STATE_TRAIN_FILTER;
				this->state_iter = 0;
				this->sub_state_iter = 0;
			} else {
				AbstractNode* filter_node = this->curr_filter->node_context.back();
				if (filter_node->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)filter_node;
					int experiment_index;
					for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
						if (action_node->experiments[e_index] == this->curr_filter) {
							experiment_index = e_index;
						}
					}
					action_node->experiments.erase(action_node->experiments.begin() + experiment_index);
				} else if (filter_node->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)filter_node;
					int experiment_index;
					for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
						if (scope_node->experiments[e_index] == this->curr_filter) {
							experiment_index = e_index;
						}
					}
					scope_node->experiments.erase(scope_node->experiments.begin() + experiment_index);
				} else {
					BranchNode* branch_node = (BranchNode*)filter_node;
					int experiment_index;
					for (int e_index = 0; e_index < (int)branch_node->experiments.size(); e_index++) {
						if (branch_node->experiments[e_index] == this->curr_filter) {
							experiment_index = e_index;
						}
					}
					branch_node->experiments.erase(branch_node->experiments.begin() + experiment_index);
					branch_node->experiment_types.erase(branch_node->experiment_types.begin() + experiment_index);
				}
				delete this->curr_filter;
				this->curr_filter = NULL;

				this->state_iter++;
				if (this->state_iter >= FIND_FILTER_ITER_LIMIT) {
					cout << "EXPERIMENT_RESULT_FAIL" << endl;
					this->result = EXPERIMENT_RESULT_FAIL;
				} else {
					cout << "SEED_EXPERIMENT_STATE_FIND_FILTER" << endl;
					this->state = SEED_EXPERIMENT_STATE_FIND_FILTER;
					create_filter();
					this->sub_state_iter = 0;
				}
			}
		}
	}
}
