/**
 * - discard explore sample
 *   - would add bias to training
 */

#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 10;
#else
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 1000;
/**
 * - need large amount
 *   - more difficult to find sample that does well on all signals in addition to true
 */
#endif /* MDEBUG */

void BranchExperiment::explore_check_activate(
		SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

	this->num_instances_until_target--;
	if (history->existing_predicted_scores.size() == 0
			&& this->num_instances_until_target <= 0) {
		vector<AbstractNode*> possible_exits;

		AbstractNode* starting_node;
		switch (this->node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->node_context;
				starting_node = start_node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				starting_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_BRANCH_END:
			{
				BranchEndNode* branch_end_node = (BranchEndNode*)this->node_context;
				starting_node = branch_end_node->next_node;
			}
			break;
		}

		this->scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		this->curr_exit_next_node = possible_exits[random_index];

		uniform_int_distribution<int> new_scope_distribution(0, 1);
		if (new_scope_distribution(generator) == 0) {
			this->curr_new_scope = create_new_scope(this->node_context->parent);
		}
		if (this->curr_new_scope != NULL) {
			this->curr_step_types.push_back(STEP_TYPE_SCOPE);
			this->curr_actions.push_back(-1);
			this->curr_scopes.push_back(this->curr_new_scope);
		} else {
			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.3);
			/**
			 * - num_steps less than exit length on average to reduce solution size
			 */
			if (random_index == 0) {
				new_num_steps = 1 + geo_distribution(generator);
			} else {
				new_num_steps = geo_distribution(generator);
			}

			vector<int> possible_child_indexes;
			for (int c_index = 0; c_index < (int)this->node_context->parent->child_scopes.size(); c_index++) {
				if (this->node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
					possible_child_indexes.push_back(c_index);
				}
			}
			uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				bool is_scope = false;
				if (possible_child_indexes.size() > 0) {
					if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
						uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
						if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
							is_scope = true;
						}
					} else {
						uniform_int_distribution<int> scope_distribution(0, 1);
						if (scope_distribution(generator) == 0) {
							is_scope = true;
						}
					}
				}
				if (is_scope) {
					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(-1);

					int child_index = possible_child_indexes[child_index_distribution(generator)];
					this->curr_scopes.push_back(this->node_context->parent->child_scopes[child_index]);
				} else {
					this->curr_step_types.push_back(STEP_TYPE_ACTION);

					this->curr_actions.push_back(-1);

					this->curr_scopes.push_back(NULL);
				}
			}
		}

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void BranchExperiment::explore_step(vector<double>& obs,
									int& action,
									bool& is_next,
									bool& fetch_action,
									SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

		this->existing_consistency_network->activate(obs);
		history->existing_predicted_consistency.push_back(this->existing_consistency_network->output->acti_vals[0]);

		this->existing_val_network->activate(obs);
		history->existing_predicted_scores.push_back(this->existing_val_network->output->acti_vals[0]);

		history->stack_traces.push_back(wrapper->scope_histories);
	}

	if (experiment_state->step_index >= (int)this->curr_step_types.size()) {
		wrapper->node_context.back() = this->curr_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->curr_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void BranchExperiment::explore_set_action(int action,
										  BranchExperimentState* experiment_state) {
	this->curr_actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void BranchExperiment::explore_exit_step(SolutionWrapper* wrapper,
										 BranchExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::explore_backprop(double target_val,
										SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	if (history->existing_predicted_scores.size() > 0) {
		double sum_vals = target_val - wrapper->solution->curr_score;
		int sum_counts = 1;

		double sum_consistency = 0.0;

		for (int l_index = 0; l_index < (int)history->stack_traces[0].size(); l_index++) {
			ScopeHistory* scope_history = history->stack_traces[0][l_index];
			Scope* scope = scope_history->scope;

			if (scope->consistency_network != NULL) {
				if (!scope_history->signal_initialized) {
					vector<double> inputs = scope_history->pre_obs;
					inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

					scope->consistency_network->activate(inputs);
					scope_history->consistency_val = scope->consistency_network->output->acti_vals[0];

					scope->pre_network->activate(scope_history->pre_obs);
					scope_history->pre_val = scope->pre_network->output->acti_vals[0];

					scope->post_network->activate(inputs);
					scope_history->post_val = scope->post_network->output->acti_vals[0];

					scope_history->signal_initialized = true;
				}

				sum_vals += (scope_history->post_val - scope_history->pre_val);
				sum_counts++;

				sum_consistency += scope_history->consistency_val;
			}

			if (scope->signal_status != SIGNAL_STATUS_FAIL) {
				int max_sample_per_timestamp = (TOTAL_MAX_SAMPLES + (int)scope->existing_pre_obs.size() - 1) / (int)scope->existing_pre_obs.size();
				if ((int)scope->explore_pre_obs.back().size() < max_sample_per_timestamp) {
					scope->explore_pre_obs.back().push_back(scope_history->pre_obs);
					scope->explore_post_obs.back().push_back(scope_history->post_obs);
				} else {
					uniform_int_distribution<int> distribution(0, scope->explore_pre_obs.back().size()-1);
					int index = distribution(generator);
					scope->explore_pre_obs.back()[index] = scope_history->pre_obs;
					scope->explore_post_obs.back()[index] = scope_history->post_obs;
				}
			}
		}

		double average_val = sum_vals / sum_counts;
		double curr_surprise = average_val - history->existing_predicted_scores[0];

		double average_consistency;
		if (sum_counts == 1) {
			average_consistency = 0.0;
		} else {
			average_consistency = sum_consistency / (sum_counts - 1);
		}
		double consistency_improvement = average_consistency - history->existing_predicted_consistency[0];

		#if defined(MDEBUG) && MDEBUG
		if ((consistency_improvement >= 0.0 && curr_surprise > this->best_surprise) || true) {
		#else
		if (consistency_improvement >= 0.0 && curr_surprise > this->best_surprise) {
		#endif /* MDEBUG */
			this->best_surprise = curr_surprise;
			if (this->best_new_scope != NULL) {
				delete this->best_new_scope;
			}
			this->best_new_scope = this->curr_new_scope;
			this->curr_new_scope = NULL;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_scopes = this->curr_scopes;
			this->best_exit_next_node = this->curr_exit_next_node;
		}

		if (this->curr_new_scope != NULL) {
			delete this->curr_new_scope;
			this->curr_new_scope = NULL;
		}
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();

		this->state_iter++;
		if (this->state_iter >= BRANCH_EXPERIMENT_EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->best_surprise >= 0.0) {
			#endif /* MDEBUG */
				uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
				this->num_instances_until_target = until_distribution(generator);

				this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
