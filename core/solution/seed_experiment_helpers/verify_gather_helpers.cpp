#include "seed_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"
#include "seed_experiment_filter.h"
#include "seed_experiment_gather.h"
#include "solution.h"

using namespace std;

void SeedExperiment::verify_gather_backprop(double target_val,
											SeedExperimentOverallHistory* history) {
	if (history->has_target) {
		if (this->sub_state_iter%2 == 0) {
			this->curr_gather_seed_score += target_val;
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (target_val > this->existing_average_score + this->existing_score_standard_deviation) {
			#endif /* MDEBUG */
				this->curr_gather_is_higher++;
			}
		} else {
			this->curr_gather_non_seed_score += target_val;
		}

		this->sub_state_iter++;
		if (this->state == SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER
				&& this->sub_state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			this->curr_gather_seed_score /= (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints / 2);
			double new_higher_ratio = (double)this->curr_gather_is_higher / (double)(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints / 2);
			this->curr_gather_non_seed_score /= (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints / 2);

			double seed_score_diff = this->curr_gather_seed_score - this->curr_filter_score;
			double seed_t_score = seed_score_diff
				/ (this->existing_score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints / 2));

			double ratio_standard_deviation = sqrt(this->curr_higher_ratio * (1 - this->curr_higher_ratio));
			double ratio_t_score = (new_higher_ratio - this->curr_higher_ratio)
				/ (ratio_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints / 2));

			double non_seed_score_diff = this->curr_gather_non_seed_score - this->existing_average_score;
			double non_seed_t_score = non_seed_score_diff
				/ (this->existing_score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints / 2));

			if (seed_t_score > -0.1 && ratio_t_score > -0.1 && non_seed_t_score > -0.1) {
			#endif /* MDEBUG */
				this->curr_gather_seed_score = 0.0;
				this->curr_gather_is_higher = 0;
				this->curr_gather_non_seed_score = 0.0;

				this->state = SEED_EXPERIMENT_STATE_VERIFY_2ND_GATHER;
				/**
				 * - leave this->state_iter unchanged
				 */
				this->sub_state_iter = 0;
			} else {
				if (this->curr_gather != NULL) {
					AbstractNode* gather_node = this->curr_gather->node_context.back();
					if (gather_node->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)gather_node;
						int experiment_index;
						for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
							if (action_node->experiments[e_index] == this->curr_gather) {
								experiment_index = e_index;
							}
						}
						action_node->experiments.erase(action_node->experiments.begin() + experiment_index);
					} else if (gather_node->type == NODE_TYPE_SCOPE) {
						ScopeNode* scope_node = (ScopeNode*)gather_node;
						int experiment_index;
						for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
							if (scope_node->experiments[e_index] == this->curr_gather) {
								experiment_index = e_index;
							}
						}
						scope_node->experiments.erase(scope_node->experiments.begin() + experiment_index);
					} else {
						BranchNode* branch_node = (BranchNode*)gather_node;
						int experiment_index;
						for (int e_index = 0; e_index < (int)branch_node->experiments.size(); e_index++) {
							if (branch_node->experiments[e_index] == this->curr_gather) {
								experiment_index = e_index;
							}
						}
						branch_node->experiments.erase(branch_node->experiments.begin() + experiment_index);
						branch_node->experiment_types.erase(branch_node->experiment_types.begin() + experiment_index);
					}
					delete this->curr_gather;
					this->curr_gather = NULL;
				}

				this->state_iter++;
				if (this->state_iter >= FIND_GATHER_ITER_LIMIT) {
					if (this->curr_filter_is_success) {
						// cout << "add filter:";
						// for (int s_index = 0; s_index < (int)this->curr_filter->filter_step_types.size(); s_index++) {
						// 	if (this->curr_filter->filter_step_types[s_index] == STEP_TYPE_ACTION) {
						// 		cout << " " << this->curr_filter->filter_actions[s_index]->action.move;
						// 	} else if (this->curr_filter->filter_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
						// 		cout << " E";
						// 	} else {
						// 		cout << " P";
						// 	}
						// }
						// cout << endl;

						this->curr_filter->add_to_scope();
						this->filters.push_back(this->curr_filter);

						this->train_filter_iter = 0;
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

						this->train_filter_iter++;
					}
					this->curr_filter = NULL;

					if (this->train_filter_iter >= TRAIN_FILTER_ITER_LIMIT
							|| this->filter_step_index == (int)this->best_step_types.size()) {
						this->result = EXPERIMENT_RESULT_FAIL;
					} else {
						this->state = SEED_EXPERIMENT_STATE_FIND_FILTER;
						this->state_iter = 0;
						create_filter();
						this->sub_state_iter = 0;
					}
				} else {
					this->state = SEED_EXPERIMENT_STATE_FIND_GATHER;
					this->sub_state_iter = -1;
				}
			}
		} else if (this->sub_state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			this->curr_gather_seed_score /= (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints / 2);
			double new_higher_ratio = (double)this->curr_gather_is_higher / (double)(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints / 2);
			this->curr_gather_non_seed_score /= (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints / 2);

			double seed_score_diff = this->curr_gather_seed_score - this->curr_filter_score;
			double seed_t_score = seed_score_diff
				/ (this->existing_score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints / 2));

			double ratio_standard_deviation = sqrt(this->curr_higher_ratio * (1 - this->curr_higher_ratio));
			double ratio_t_score = (new_higher_ratio - this->curr_higher_ratio)
				/ (ratio_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints / 2));

			double non_seed_score_diff = this->curr_gather_non_seed_score - this->existing_average_score;
			double non_seed_t_score = non_seed_score_diff
				/ (this->existing_score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints / 2));

			if (seed_t_score > -0.1 && ratio_t_score > -0.1 && non_seed_t_score > -0.1) {
			#endif /* MDEBUG */
				this->curr_gather->add_to_scope();

				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_is_higher_histories.reserve(solution->curr_num_datapoints);

				this->state = SEED_EXPERIMENT_STATE_TRAIN_FILTER;
				this->state_iter = 0;
				this->sub_state_iter = 0;
			} else {
				if (this->curr_gather != NULL) {
					AbstractNode* gather_node = this->curr_gather->node_context.back();
					if (gather_node->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)gather_node;
						int experiment_index;
						for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
							if (action_node->experiments[e_index] == this->curr_gather) {
								experiment_index = e_index;
							}
						}
						action_node->experiments.erase(action_node->experiments.begin() + experiment_index);
					} else if (gather_node->type == NODE_TYPE_SCOPE) {
						ScopeNode* scope_node = (ScopeNode*)gather_node;
						int experiment_index;
						for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
							if (scope_node->experiments[e_index] == this->curr_gather) {
								experiment_index = e_index;
							}
						}
						scope_node->experiments.erase(scope_node->experiments.begin() + experiment_index);
					} else {
						BranchNode* branch_node = (BranchNode*)gather_node;
						int experiment_index;
						for (int e_index = 0; e_index < (int)branch_node->experiments.size(); e_index++) {
							if (branch_node->experiments[e_index] == this->curr_gather) {
								experiment_index = e_index;
							}
						}
						branch_node->experiments.erase(branch_node->experiments.begin() + experiment_index);
						branch_node->experiment_types.erase(branch_node->experiment_types.begin() + experiment_index);
					}
					delete this->curr_gather;
					this->curr_gather = NULL;
				}

				this->state_iter++;
				if (this->state_iter >= FIND_GATHER_ITER_LIMIT) {
					if (this->curr_filter_is_success) {
						// cout << "add filter:";
						// for (int s_index = 0; s_index < (int)this->curr_filter->filter_step_types.size(); s_index++) {
						// 	if (this->curr_filter->filter_step_types[s_index] == STEP_TYPE_ACTION) {
						// 		cout << " " << this->curr_filter->filter_actions[s_index]->action.move;
						// 	} else if (this->curr_filter->filter_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
						// 		cout << " E";
						// 	} else {
						// 		cout << " P";
						// 	}
						// }
						// cout << endl;

						this->curr_filter->add_to_scope();
						this->filters.push_back(this->curr_filter);

						this->train_filter_iter = 0;
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

						this->train_filter_iter++;
					}
					this->curr_filter = NULL;

					if (this->train_filter_iter >= TRAIN_FILTER_ITER_LIMIT
							|| this->filter_step_index == (int)this->best_step_types.size()) {
						this->result = EXPERIMENT_RESULT_FAIL;
					} else {
						this->state = SEED_EXPERIMENT_STATE_FIND_FILTER;
						this->state_iter = 0;
						create_filter();
						this->sub_state_iter = 0;
					}
				} else {
					this->state = SEED_EXPERIMENT_STATE_FIND_GATHER;
					this->sub_state_iter = -1;
				}
			}
		}
	}
}
