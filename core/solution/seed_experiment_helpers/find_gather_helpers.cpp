#include "seed_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "scope_node.h"
#include "seed_experiment_filter.h"
#include "seed_experiment_gather.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_SAMPLES_PER_ITER = 4;
const int CURR_GATHER_EXCEEDED_LIMIT_LIMIT = 2;
#else
const int NUM_SAMPLES_PER_ITER = 40;
const int CURR_GATHER_EXCEEDED_LIMIT_LIMIT = 5;
#endif /* MDEBUG */

void SeedExperiment::find_gather_backprop(double target_val,
										  RunHelper& run_helper,
										  SeedExperimentOverallHistory* history) {
	if (this->curr_gather != NULL) {
		if (run_helper.exceeded_limit) {
			this->curr_gather_exceeded_limit_count++;
			if (this->curr_gather_exceeded_limit_count >= CURR_GATHER_EXCEEDED_LIMIT_LIMIT) {
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

				this->state_iter++;
				if (this->state_iter >= FIND_GATHER_ITER_LIMIT) {
					if (this->curr_filter_is_success) {
						this->curr_filter->add_to_scope();
						this->filters.push_back(this->curr_filter);
						this->filter_step_index = this->curr_filter_step_index;
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
					}
					this->curr_filter = NULL;

					if (this->filter_step_index == (int)this->best_step_types.size()) {
						cout << "EXPERIMENT_RESULT_FAIL" << endl;
						this->result = EXPERIMENT_RESULT_FAIL;
					} else {
						cout << "SEED_EXPERIMENT_STATE_FIND_FILTER" << endl;
						this->state = SEED_EXPERIMENT_STATE_FIND_FILTER;
						this->state_iter = 0;
						create_filter();
						this->sub_state_iter = 0;
					}
				} else {
					this->sub_state_iter = -1;
				}

				return;
			}
		} else {
			this->curr_gather_exceeded_limit_count = 0;
		}

		if (history->has_target) {
			if (this->sub_state_iter != -1) {
				if (this->sub_state_iter%2 == 0) {
					if (target_val > this->existing_average_score + this->existing_score_standard_deviation) {
						this->curr_gather_is_higher++;
					}
				} else {
					this->curr_gather_score += target_val;
				}
			}

			this->sub_state_iter++;
			if (this->sub_state_iter >= NUM_SAMPLES_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				double new_higher_ratio = (double)this->curr_gather_is_higher / (double)(NUM_SAMPLES_PER_ITER / 2);
				this->curr_gather_score /= (NUM_SAMPLES_PER_ITER / 2);

				double ratio_standard_deviation = sqrt(this->curr_higher_ratio * (1 - this->curr_higher_ratio));
				double ratio_t_score = (new_higher_ratio - this->curr_higher_ratio)
					/ (ratio_standard_deviation / sqrt(NUM_SAMPLES_PER_ITER / 2));

				double score_diff = this->curr_gather_score - this->existing_average_score;
				double t_score = score_diff
					/ (this->existing_score_standard_deviation / sqrt(NUM_SAMPLES_PER_ITER / 2));

				if (ratio_t_score > -0.2 && t_score > -0.2) {
				#endif /* MDEBUG */
					cout << "SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER" << endl;
					this->state = SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER;
					/**
					 * - leave this->state_iter unchanged
					 */
					this->sub_state_iter = 0;
				} else {
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

					this->state_iter++;
					if (this->state_iter >= FIND_GATHER_ITER_LIMIT) {
						if (this->curr_filter_is_success) {
							this->curr_filter->add_to_scope();
							this->filters.push_back(this->curr_filter);
							this->filter_step_index = this->curr_filter_step_index;
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
						}
						this->curr_filter = NULL;

						if (this->filter_step_index == (int)this->best_step_types.size()) {
							cout << "EXPERIMENT_RESULT_FAIL" << endl;
							this->result = EXPERIMENT_RESULT_FAIL;
						} else {
							cout << "SEED_EXPERIMENT_STATE_FIND_FILTER" << endl;
							this->state = SEED_EXPERIMENT_STATE_FIND_FILTER;
							this->state_iter = 0;
							create_filter();
							this->sub_state_iter = 0;
						}
					} else {
						this->sub_state_iter = -1;
					}
				}
			}
		}
	}
}
