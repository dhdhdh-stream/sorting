#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 5;
#else
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 200;
#endif /* MDEBUG */

void BranchExperiment::explore_check_activate(
		SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (wrapper->experiment_instance_histories.size() == 0
			&& this->num_instances_until_target == 0) {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		BranchExperimentInstanceHistory* instance_history = new BranchExperimentInstanceHistory(this);
		wrapper->experiment_instance_histories.push_back(instance_history);

		double sum_vals = this->existing_average_score;
		for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   this->existing_inputs[i_index],
							   0,
							   val,
							   is_on);
			if (is_on) {
				double normalized_val = (val - this->existing_input_averages[i_index]) / this->existing_input_standard_deviations[i_index];
				sum_vals += this->existing_weights[i_index] * normalized_val;
			}
		}
		instance_history->existing_predicted_score = sum_vals;

		for (int l_index = (int)wrapper->scope_histories.size()-1; l_index >= 0; l_index--) {
			if (wrapper->scope_histories[l_index]->scope->signals.size() > 0) {
				instance_history->signal_needed_from = wrapper->scope_histories[l_index];
				break;
			}
		}

		this->curr_scope_history = new ScopeHistory(scope_history);

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
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;
				starting_node = obs_node->next_node;
			}
			break;
		}

		this->scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		int random_index;
		geometric_distribution<int> exit_distribution(0.2);
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		this->curr_exit_next_node = possible_exits[random_index];

		#if defined(MDEBUG) && MDEBUG
		uniform_int_distribution<int> new_scope_distribution(0, 1);
		#else
		uniform_int_distribution<int> new_scope_distribution(0, 4);
		#endif /* MDEBUG */
		if (new_scope_distribution(generator) == 0) {
			this->curr_new_scope = create_new_scope(this->scope_context);
		}
		if (this->curr_new_scope != NULL) {
			this->curr_step_types.push_back(STEP_TYPE_SCOPE);
			this->curr_actions.push_back(-1);
			this->curr_scopes.push_back(this->curr_new_scope);
		} else {
			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.2);
			if (random_index == 0) {
				new_num_steps = 1 + geo_distribution(generator);
			} else {
				new_num_steps = geo_distribution(generator);
			}

			/**
			 * - always give raw actions a large weight
			 *   - existing scopes often learned to avoid certain patterns
			 *     - which can prevent innovation
			 */
			uniform_int_distribution<int> scope_distribution(0, 1);
			vector<Scope*> possible_scopes;
			for (int c_index = 0; c_index < (int)this->scope_context->child_scopes.size(); c_index++) {
				if (this->scope_context->child_scopes[c_index]->nodes.size() > 1) {
					possible_scopes.push_back(this->scope_context->child_scopes[c_index]);
				}
			}
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				if (scope_distribution(generator) == 0 && possible_scopes.size() > 0) {
					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(-1);

					uniform_int_distribution<int> child_scope_distribution(0, possible_scopes.size()-1);
					this->curr_scopes.push_back(possible_scopes[child_scope_distribution(generator)]);
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
}

void BranchExperiment::explore_step(vector<double>& obs,
									int& action,
									bool& is_next,
									bool& fetch_action,
									SolutionWrapper* wrapper,
									BranchExperimentState* experiment_state) {
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

void BranchExperiment::explore_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1);
	this->num_instances_until_target = 1 + until_distribution(generator);

	if (wrapper->experiment_instance_histories.size() > 0) {
		BranchExperimentInstanceHistory* instance_history =
			(BranchExperimentInstanceHistory*)wrapper->experiment_instance_histories[0];

		double inner_targel_val;
		if (instance_history->signal_needed_from == NULL) {
			inner_targel_val = target_val;
		} else {
			inner_targel_val = calc_signal(instance_history->signal_needed_from);
		}

		double curr_surprise = inner_targel_val - instance_history->existing_predicted_score;

		#if defined(MDEBUG) && MDEBUG
		if (true) {
		#else
		if (curr_surprise > this->best_surprise) {
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
			if (this->best_scope_history != NULL) {
				delete this->best_scope_history;
			}
			this->best_scope_history = this->curr_scope_history;
			this->curr_scope_history = NULL;
		}

		if (this->curr_new_scope != NULL) {
			delete this->curr_new_scope;
			this->curr_new_scope = NULL;
		}
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();
		if (this->curr_scope_history != NULL) {
			delete this->curr_scope_history;
			this->curr_scope_history = NULL;
		}

		this->state_iter++;
		if (this->state_iter >= BRANCH_EXPERIMENT_EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (this->best_surprise > 0.0 || true) {
			#else
			if (this->best_surprise > 0.0) {
			#endif /* MDEBUG */
				uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
				this->num_instances_until_target = 1 + until_distribution(generator);

				this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
