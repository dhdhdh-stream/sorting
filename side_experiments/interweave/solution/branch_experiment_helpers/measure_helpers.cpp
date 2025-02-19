#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_DATAPOINTS = 20;
#else
const int MEASURE_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void BranchExperiment::measure_activate(AbstractNode*& curr_node,
										Problem* problem,
										RunHelper& run_helper,
										ScopeHistory* scope_history,
										BranchExperimentOverallHistory* overall_history) {
	BranchExperimentInstanceHistory* instance_history = new BranchExperimentInstanceHistory(this);
	run_helper.instance_histories.push_back(instance_history);

	uniform_int_distribution<int> active_distribution(
		-1, overall_history->active_concurrents.size()-1);
	int concurrent_index = overall_history->active_concurrents[
		active_distribution(generator)];
	instance_history->concurrent_index = concurrent_index;

	if (concurrent_index != -1) {
		if (this->select_percentage[concurrent_index] == 1.0) {
			for (int s_index = 0; s_index < (int)this->step_types[concurrent_index].size(); s_index++) {
				if (this->step_types[concurrent_index][s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->actions[concurrent_index][s_index]);
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[concurrent_index][s_index]);
					this->scopes[concurrent_index][s_index]->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}

				run_helper.num_actions += 2;
			}

			curr_node = this->exit_next_node[concurrent_index];
		} else {
			double sum_vals = this->new_average_score[concurrent_index];
			for (int f_index = 0; f_index < (int)this->new_factor_ids[concurrent_index].size(); f_index++) {
				double val;
				fetch_factor_helper(scope_history,
									this->new_factor_ids[concurrent_index][f_index],
									val);
				sum_vals += this->new_factor_weights[concurrent_index][f_index] * val;
			}

			bool is_branch;
			if (sum_vals >= 0.0) {
				is_branch = true;
			} else {
				is_branch = false;
			}

			if (is_branch) {
				for (int s_index = 0; s_index < (int)this->step_types[concurrent_index].size(); s_index++) {
					if (this->step_types[concurrent_index][s_index] == STEP_TYPE_ACTION) {
						problem->perform_action(this->actions[concurrent_index][s_index]);
					} else {
						ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[concurrent_index][s_index]);
						this->scopes[concurrent_index][s_index]->activate(problem,
							run_helper,
							inner_scope_history);
						delete inner_scope_history;
					}

					run_helper.num_actions += 2;
				}

				curr_node = this->exit_next_node[concurrent_index];
			}
		}
	}

	this->instance_iter++;
}

void BranchExperiment::measure_backprop(
		BranchExperimentInstanceHistory* instance_history,
		double target_val) {
	this->combined_scores[instance_history->concurrent_index+1] += target_val;
	this->combined_counts[instance_history->concurrent_index+1]++;
}

void BranchExperiment::measure_update() {
	this->run_iter++;
	if (this->run_iter >= MEASURE_NUM_DATAPOINTS
			&& this->instance_iter >= MEASURE_NUM_DATAPOINTS * (BRANCH_EXPERIMENT_NUM_CONCURRENT + 1)) {
		int best_index = 0;
		double best_score = this->combined_scores[0] / this->combined_counts[0];
		for (int c_index = 1; c_index < (int)this->combined_scores.size(); c_index++) {
			double curr_score = this->combined_scores[c_index] / this->combined_counts[c_index];
			if (curr_score > best_score) {
				best_index = c_index;
				best_score = curr_score;
			}
		}

		if (best_index == 0) {
			this->result = EXPERIMENT_RESULT_FAIL;
		} else {
			this->best_concurrent_index = best_index-1;

			cout << "BranchExperiment success" << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->step_types[this->best_concurrent_index].size(); s_index++) {
				if (this->step_types[this->best_concurrent_index][s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->actions[this->best_concurrent_index][s_index].move;
				} else {
					cout << " E" << this->scopes[this->best_concurrent_index][s_index]->id;
				}
			}
			cout << endl;
			if (this->exit_next_node[this->best_concurrent_index] == NULL) {
				cout << "this->exit_next_node[this->best_concurrent_index]->id: " << -1 << endl;
			} else {
				cout << "this->exit_next_node[this->best_concurrent_index]->id: " << this->exit_next_node[this->best_concurrent_index]->id << endl;
			}

			double existing_score = this->combined_scores[0] / this->combined_counts[0];
			cout << "existing_score: " << existing_score << endl;

			double best_score = this->combined_scores[this->best_concurrent_index+1]
				/ this->combined_counts[this->best_concurrent_index+1];
			cout << "best_score: " << best_score << endl;

			cout << endl;

			this->result = EXPERIMENT_RESULT_SUCCESS;
		}
	}
}
