#include "seed_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope_node.h"
#include "seed_experiment_filter.h"
#include "seed_experiment_gather.h"
#include "solution.h"

using namespace std;

void SeedExperiment::measure_filter_backprop(double target_val,
											 SeedExperimentOverallHistory* history) {
	if (history->has_target) {
		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (target_val > this->existing_average_score + this->existing_score_standard_deviation) {
		#endif /* MDEBUG */
			this->i_is_higher_histories.push_back(true);
		} else {
			this->i_is_higher_histories.push_back(false);
		}

		if ((int)this->i_is_higher_histories.size() >= solution->curr_num_datapoints) {
			int num_seed = 0;
			int num_higher = 0;
			for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
				if (this->i_is_seed_histories[d_index]) {
					if (this->i_is_higher_histories[d_index]) {
						num_higher++;
					}
					num_seed++;
				}
			}
			double seed_ratio = (double)num_seed / (double)solution->curr_num_datapoints;
			double higher_ratio = (double)num_higher / (double)num_seed;

			this->i_is_seed_histories.clear();
			this->i_is_higher_histories.clear();

			#if defined(MDEBUG) && MDEBUG
			if (seed_ratio > 0.0 && rand()%2 == 0) {
			#else
			double improve_threshold;
			if (this->curr_higher_ratio > 0.5) {
				improve_threshold = 0.1 * (1.0 - this->curr_higher_ratio);
			} else {
				improve_threshold = 0.1 * this->curr_higher_ratio;
			}

			// temp
			if (this->curr_gather != NULL) {
				cout << "seed_ratio: " << seed_ratio << endl;
				cout << "higher_ratio: " << higher_ratio << endl;
			}

			if (seed_ratio > 0.0 && higher_ratio > this->curr_higher_ratio + improve_threshold) {
			#endif /* MDEBUG */
				this->curr_filter->input_scope_contexts = this->curr_filter->test_input_scope_contexts;
				this->curr_filter->test_input_scope_contexts.clear();
				this->curr_filter->input_node_contexts = this->curr_filter->test_input_node_contexts;
				this->curr_filter->test_input_node_contexts.clear();
				this->curr_filter->network_input_indexes = this->curr_filter->test_network_input_indexes;
				this->curr_filter->test_network_input_indexes.clear();
				if (this->curr_filter->network != NULL) {
					delete this->curr_filter->network;
				}
				this->curr_filter->network = this->curr_filter->test_network;
				this->curr_filter->test_network = NULL;
				this->curr_filter->average_misguess = this->curr_filter->test_average_misguess;
				this->curr_filter->misguess_standard_deviation = this->curr_filter->test_misguess_standard_deviation;

				this->curr_higher_ratio = higher_ratio;

				if (this->curr_gather != NULL) {
					// cout << "add gather:";
					// for (int s_index = 0; s_index < (int)this->curr_gather->step_types.size(); s_index++) {
					// 	if (this->curr_gather->step_types[s_index] == STEP_TYPE_ACTION) {
					// 		cout << " " << this->curr_gather->actions[s_index]->action.move;
					// 	} else if (this->curr_gather->step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
					// 		cout << " E";
					// 	} else {
					// 		cout << " P";
					// 	}
					// }
					// cout << endl;

					this->existing_average_score = this->curr_gather_existing_average_score;
					this->existing_score_standard_deviation = this->curr_gather_existing_score_standard_deviation;

					this->gathers.push_back(this->curr_gather);
					this->curr_gather = NULL;

					this->train_gather_iter = 0;

					cout << "SEED_EXPERIMENT_STATE_MEASURE" << endl;
				}

				this->curr_filter_is_success = true;

				this->combined_score = 0.0;

				this->state = SEED_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->curr_filter->test_input_scope_contexts.clear();
				this->curr_filter->test_input_node_contexts.clear();
				this->curr_filter->test_network_input_indexes.clear();
				if (this->curr_filter->test_network != NULL) {
					delete this->curr_filter->test_network;
					this->curr_filter->test_network = NULL;
				}

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

				this->train_gather_iter++;
				if (this->train_gather_iter >= TRAIN_GATHER_ITER_LIMIT) {
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
					this->state_iter = 0;
				}
			}
		}
	}
}
