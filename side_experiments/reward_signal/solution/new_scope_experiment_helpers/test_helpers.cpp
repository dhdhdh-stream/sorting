#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewScopeExperiment::test_backprop(
		double target_val,
		NewScopeExperimentHistory* history) {
	this->test_target_val_histories.push_back(target_val);

	if ((int)this->test_target_val_histories.size() == EARLY_SUCCESS_S1_ITERS
			|| (int)this->test_target_val_histories.size() == EARLY_SUCCESS_S2_ITERS) {
		double sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->test_target_val_histories.size(); h_index++) {
			sum_score += this->test_target_val_histories[h_index];
		}
		double new_score = sum_score / (double)this->test_target_val_histories.size();

		double sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->test_target_val_histories.size(); h_index++) {
			sum_variance += (this->test_target_val_histories[h_index] - new_score)
				* (this->test_target_val_histories[h_index] - new_score);
		}
		double new_standard_deviation = sqrt(sum_variance / (double)this->test_target_val_histories.size());
		if (new_standard_deviation < MIN_STANDARD_DEVIATION) {
			new_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		double existing_score;
		switch (test_location_start->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->test_location_start;
				if (this->successful_location_starts.size() == 0) {
					existing_score = action_node->average_score;
				} else {
					existing_score = action_node->new_scope_average_score;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->test_location_start;
				if (this->successful_location_starts.size() == 0) {
					existing_score = scope_node->average_score;
				} else {
					existing_score = scope_node->new_scope_average_score;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->test_location_start;
				if (this->successful_location_starts.size() == 0) {
					if (this->test_location_is_branch) {
						existing_score = branch_node->branch_average_score;
					} else {
						existing_score = branch_node->original_average_score;
					}
				} else {
					if (this->test_location_is_branch) {
						existing_score = branch_node->branch_new_scope_average_score;
					} else {
						existing_score = branch_node->original_new_scope_average_score;
					}
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->test_location_start;
				if (this->successful_location_starts.size() == 0) {
					existing_score = obs_node->average_score;
				} else {
					existing_score = obs_node->new_scope_average_score;
				}
			}
			break;
		}

		double t_score = (new_score - existing_score) / new_standard_deviation;
		if (t_score >= EARLY_SUCCESS_MIN_T_SCORE) {
			if (this->successful_location_starts.size() == 0) {
				this->new_score = new_score;
			}

			/**
			 * - simply set to 1.0 to allow follow up
			 */
			this->test_scope_node->new_scope_average_hits_per_run = 1.0;
			this->test_scope_node->new_scope_average_score = new_score;

			this->successful_location_starts.push_back(this->test_location_start);
			this->successful_location_is_branch.push_back(this->test_location_is_branch);
			this->successful_scope_nodes.push_back(this->test_scope_node);
			this->test_scope_node = NULL;

			this->scope_context->new_scope_measure_update(this->state_iter);

			this->test_location_start = NULL;

			double improvement = new_score - existing_score;
			cout << "improvement: " << improvement << endl;

			if (this->successful_location_starts.size() >= NEW_SCOPE_NUM_LOCATIONS) {
				cout << "NewScopeExperiment success" << endl;

				this->result = EXPERIMENT_RESULT_SUCCESS;
			}
		}
	} else if ((int)this->test_target_val_histories.size() == MEASURE_S1_ITERS
			|| (int)this->test_target_val_histories.size() == MEASURE_S2_ITERS
			|| (int)this->test_target_val_histories.size() == MEASURE_S3_ITERS
			|| (int)this->test_target_val_histories.size() == MEASURE_S4_ITERS) {
		double sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->test_target_val_histories.size(); h_index++) {
			sum_score += this->test_target_val_histories[h_index];
		}
		double new_score = sum_score / (double)this->test_target_val_histories.size();

		double existing_score;
		switch (test_location_start->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->test_location_start;
				if (this->successful_location_starts.size() == 0) {
					existing_score = action_node->average_score;
				} else {
					existing_score = action_node->new_scope_average_score;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->test_location_start;
				if (this->successful_location_starts.size() == 0) {
					existing_score = scope_node->average_score;
				} else {
					existing_score = scope_node->new_scope_average_score;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->test_location_start;
				if (this->successful_location_starts.size() == 0) {
					if (this->test_location_is_branch) {
						existing_score = branch_node->branch_average_score;
					} else {
						existing_score = branch_node->original_average_score;
					}
				} else {
					if (this->test_location_is_branch) {
						existing_score = branch_node->branch_new_scope_average_score;
					} else {
						existing_score = branch_node->original_new_scope_average_score;
					}
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->test_location_start;
				if (this->successful_location_starts.size() == 0) {
					existing_score = obs_node->average_score;
				} else {
					existing_score = obs_node->new_scope_average_score;
				}
			}
			break;
		}
		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (new_score < existing_score) {
		#endif /* MDEBUG */
			this->test_location_start->experiment = NULL;
			this->test_location_start = NULL;
			delete this->test_scope_node;
			this->test_scope_node = NULL;

			if (this->generalize_iter == -1
					&& this->successful_location_starts.size() == 0) {
				this->result = EXPERIMENT_RESULT_FAIL;
				/**
				 * - only continue if first succeeds
				 */
			} else {
				this->generalize_iter++;
				if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
					this->result = EXPERIMENT_RESULT_FAIL;
				}
			}
		} else if ((int)this->test_target_val_histories.size() == MEASURE_S4_ITERS) {
			if (this->successful_location_starts.size() == 0) {
				this->new_score = new_score;
			}

			/**
			 * - simply set to 1.0 to allow follow up
			 */
			this->test_scope_node->new_scope_average_hits_per_run = 1.0;
			this->test_scope_node->new_scope_average_score = new_score;

			this->successful_location_starts.push_back(this->test_location_start);
			this->successful_location_is_branch.push_back(this->test_location_is_branch);
			this->successful_scope_nodes.push_back(this->test_scope_node);
			this->test_scope_node = NULL;

			this->scope_context->new_scope_measure_update(this->state_iter);

			this->test_location_start = NULL;

			double improvement = new_score - existing_score;
			cout << "improvement: " << improvement << endl;

			if (this->successful_location_starts.size() >= NEW_SCOPE_NUM_LOCATIONS) {
				cout << "NewScopeExperiment success" << endl;

				this->result = EXPERIMENT_RESULT_SUCCESS;
			}
		}
	}
}
