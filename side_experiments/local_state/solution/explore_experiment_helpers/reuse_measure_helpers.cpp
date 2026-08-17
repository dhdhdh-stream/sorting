#include "explore_experiment.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "noop_node.h"
#include "obs_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void ExploreExperiment::reuse_measure_check_activate(
		vector<double>& obs,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXPLORE) {
		bool is_branch;
		this->existing_network->activate(wrapper->states.back());
		this->new_network->activate(wrapper->states.back());
		if (this->new_network->output->acti_vals(0) >= this->existing_network->output->acti_vals(0)) {
			is_branch = true;
		} else {
			is_branch = false;
		}

		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#endif /* MDEBUG */

		if (is_branch) {
			ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void ExploreExperiment::reuse_measure_step(vector<double>& obs,
										   int& action,
										   bool& is_next,
										   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_indexes[experiment_state->step_index];
			is_next = true;

			wrapper->run_num_actions++;
		} else {
			ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[
				this->best_indexes[experiment_state->step_index]];
			generic_scope_node->experiment_step(obs,
												action,
												is_next,
												wrapper);
		}
	}
}

void ExploreExperiment::reuse_measure_callback(vector<double>& obs,
											   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	int action = this->best_indexes[experiment_state->step_index];
	ActionNode* generic_action_node = this->scope_context->generic_action_nodes[action];
	generic_action_node->experiment_step_callback(obs,
												  wrapper);

	experiment_state->step_index++;
}

void ExploreExperiment::reuse_measure_exit_step(vector<double>& obs,
												SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[
		this->best_indexes[experiment_state->step_index]];
	generic_scope_node->experiment_exit_step(obs,
											 wrapper);

	experiment_state->step_index++;
}

void ExploreExperiment::reuse_measure_backprop(double target_val,
											   ExploreExperimentHistory* history,
											   SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXPLORE) {
		this->sum_vals += target_val;

		this->state_iter++;
		if (this->state_iter >= EXPERIMENT_MEASURE_NUM_DATAPOINTS) {
			double new_val_average = this->sum_vals / this->state_iter;

			double local_improvement = new_val_average - this->existing_val_average;

			double average_hits_per_run;
			switch (this->node_context->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)this->node_context;
					average_hits_per_run = noop_node->average_instances_per_run / noop_node->average_instances_per_hit;
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					average_hits_per_run = action_node->average_instances_per_run / action_node->average_instances_per_hit;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					average_hits_per_run = scope_node->average_instances_per_run / scope_node->average_instances_per_hit;
				}
				break;
			default:
			// case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						average_hits_per_run = branch_node->branch_average_instances_per_run / branch_node->branch_average_instances_per_hit;
					} else {
						average_hits_per_run = branch_node->original_average_instances_per_run / branch_node->original_average_instances_per_hit;
					}
				}
				break;
			}
			double global_improvement = average_hits_per_run * local_improvement;

			// // temp
			// cout << "measure reuse" << endl;
			// cout << "this->scope_context->id: " << this->scope_context->id << endl;
			// cout << "local_improvement: " << local_improvement << endl;
			// cout << "global_improvement: " << global_improvement << endl;

			bool is_success = false;
			if (local_improvement > 0.0) {
				if (this->scope_context->measure_reuse_last_scores.size() >= MIN_NUM_LAST_TRACK) {
					int num_better_than = 0;
					for (list<double>::iterator it = this->scope_context->measure_reuse_last_scores.begin();
							it != this->scope_context->measure_reuse_last_scores.end(); it++) {
						if (global_improvement >= *it) {
							num_better_than++;
						}
					}

					double target_better_than = LAST_BETTER_THAN_RATIO * (double)this->scope_context->measure_reuse_last_scores.size();

					if (num_better_than >= target_better_than) {
						is_success = true;
					}

					if (this->scope_context->measure_reuse_last_scores.size() >= NUM_LAST_TRACK) {
						this->scope_context->measure_reuse_last_scores.pop_front();
					}
					this->scope_context->measure_reuse_last_scores.push_back(global_improvement);
				} else {
					this->scope_context->measure_reuse_last_scores.push_back(global_improvement);
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if (is_success || rand()%3 != 0) {
			#else
			if (is_success) {
			#endif /* MDEBUG */
				add(false,
					wrapper);
			} else {
				delete this;
			}
		}
	}
}
