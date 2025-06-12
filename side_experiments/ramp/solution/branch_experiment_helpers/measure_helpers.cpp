#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

// #if defined(MDEBUG) && MDEBUG
// const int MEASURE_NUM_DATAPOINTS = 20;
// #else
// const int MEASURE_NUM_DATAPOINTS = 4000;
// #endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int MEASURE_1_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_5_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_10_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_25_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_50_PERCENT_NUM_DATAPOINTS = 2;
#else
const int MEASURE_1_PERCENT_NUM_DATAPOINTS = 10;
const int MEASURE_5_PERCENT_NUM_DATAPOINTS = 50;
const int MEASURE_10_PERCENT_NUM_DATAPOINTS = 100;
const int MEASURE_25_PERCENT_NUM_DATAPOINTS = 250;
const int MEASURE_50_PERCENT_NUM_DATAPOINTS = 500;
#endif /* MDEBUG */

void BranchExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	if (history->is_active) {
		if (this->select_percentage == 1.0) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->best_actions[s_index]);

					run_helper.num_actions++;
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
					this->best_scopes[s_index]->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}
			}

			curr_node = this->best_exit_next_node;
		} else {
			double sum_vals = this->new_average_score;
			for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
				double val;
				fetch_factor_helper(scope_history,
									this->new_factor_ids[f_index],
									val);
				sum_vals += this->new_factor_weights[f_index] * val;
			}

			bool decision_is_branch;
			if (sum_vals >= 0.0) {
				decision_is_branch = true;
			} else {
				decision_is_branch = false;
			}

			if (decision_is_branch) {
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						problem->perform_action(this->best_actions[s_index]);

						run_helper.num_actions++;
					} else {
						ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
						this->best_scopes[s_index]->activate(problem,
							run_helper,
							inner_scope_history);
						delete inner_scope_history;
					}
				}

				curr_node = this->best_exit_next_node;
			}
		}
	}
}

void BranchExperiment::measure_backprop(double target_val,
										BranchExperimentHistory* history,
										set<Scope*>& updated_scopes) {
	if (history->is_active) {
		this->combined_sum_score += target_val;
		this->combined_count++;
	} else {
		this->existing_sum_score += target_val;
		this->existing_count++;
	}

	this->state_iter++;
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_MEASURE_1_PERCENT:
		if (this->combined_count >= MEASURE_1_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double combined_score = this->combined_sum_score / this->combined_count;
			if (combined_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->combined_sum_score = 0.0;
				this->combined_count = 0;

				this->state = BRANCH_EXPERIMENT_STATE_MEASURE_5_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE_5_PERCENT:
		if (this->combined_count >= MEASURE_5_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double combined_score = this->combined_sum_score / this->combined_count;
			if (combined_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->combined_sum_score = 0.0;
				this->combined_count = 0;

				this->state = BRANCH_EXPERIMENT_STATE_MEASURE_10_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE_10_PERCENT:
		if (this->combined_count >= MEASURE_10_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double combined_score = this->combined_sum_score / this->combined_count;
			if (combined_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->combined_sum_score = 0.0;
				this->combined_count = 0;

				this->state = BRANCH_EXPERIMENT_STATE_MEASURE_25_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE_25_PERCENT:
		if (this->combined_count >= MEASURE_25_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double combined_score = this->combined_sum_score / this->combined_count;
			if (combined_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->combined_sum_score = 0.0;
				this->combined_count = 0;

				this->state = BRANCH_EXPERIMENT_STATE_MEASURE_50_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE_50_PERCENT:
		if (this->combined_count >= MEASURE_50_PERCENT_NUM_DATAPOINTS) {
			double existing_score = this->existing_sum_score / this->existing_count;
			double combined_score = this->combined_sum_score / this->combined_count;
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			if (combined_score > existing_score) {
			#endif /* MDEBUG */
				cout << "BranchExperiment" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index].move;
					} else {
						cout << " E" << this->best_scopes[s_index]->id;
					}
				}
				cout << endl;

				if (this->best_exit_next_node == NULL) {
					cout << "this->best_exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				}

				cout << "this->select_percentage: " << this->select_percentage << endl;

				double improvement = combined_score - existing_score;
				cout << "improvement: " << improvement << endl;

				cout << endl;

				add();
				updated_scopes.insert(this->scope_context);
				delete this;
			} else {
				delete this;
				return;
			}
		}
		break;
	}
}
