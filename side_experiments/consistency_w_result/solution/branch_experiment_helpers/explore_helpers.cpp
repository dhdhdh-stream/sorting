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
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 4000;
/**
 * - need large amount
 *   - more difficult to find sample that does well on all signals in addition to true
 */
#endif /* MDEBUG */

void BranchExperiment::explore_result_check_activate(
		SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

	uniform_int_distribution<int> select_distribution(0, history->num_instances);
	if (select_distribution(generator) == 0) {
		history->explore_index = history->num_instances;
		history->stack_trace = wrapper->scope_histories;
	}
	history->num_instances++;
}

void BranchExperiment::explore_result_backprop(SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

	for (int l_index = 0; l_index < (int)history->stack_trace.size(); l_index++) {
		ScopeHistory* scope_history = history->stack_trace[l_index];
		Scope* scope = scope_history->scope;
		if (scope->consistency_network == NULL) {
			history->existing_predicted_consistency.push_back(0.0);
		} else {
			vector<double> input = scope_history->pre_obs;
			input.insert(input.end(), scope_history->post_obs.begin(),
				scope_history->post_obs.end());

			scope->consistency_network->activate(input);
			history->existing_predicted_consistency.push_back(
				scope->consistency_network->output->acti_vals[0]);
		}
	}

	history->num_instances = 0;
}

void BranchExperiment::explore_check_activate(
		SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

	if (history->num_instances == history->explore_index) {
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

		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}

	history->num_instances++;
}

void BranchExperiment::explore_step(vector<double>& obs,
									int& action,
									bool& is_next,
									bool& fetch_action,
									SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

		history->stack_trace = wrapper->scope_histories;
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

	bool is_consistent = true;
	for (int l_index = 0; l_index < (int)history->stack_trace.size(); l_index++) {
		ScopeHistory* scope_history = history->stack_trace[l_index];
		Scope* scope = scope_history->scope;
		if (scope->consistency_network != NULL) {
			vector<double> inputs = scope_history->pre_obs;
			inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

			scope->consistency_network->activate(inputs);

			if (scope->consistency_network->output->acti_vals[0] < history->existing_predicted_consistency[l_index]) {
				is_consistent = false;
			}
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

	double curr_surprise = target_val - wrapper->existing_result;

	#if defined(MDEBUG) && MDEBUG
	if ((is_consistent && curr_surprise > this->best_surprise) || true) {
	#else
	if (is_consistent && curr_surprise > this->best_surprise) {
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
			this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
