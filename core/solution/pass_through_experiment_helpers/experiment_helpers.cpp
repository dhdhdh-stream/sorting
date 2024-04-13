#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::experiment_activate(AbstractNode*& curr_node,
												vector<ContextLayer>& context,
												RunHelper& run_helper,
												PassThroughExperimentHistory* history) {
	if (this->parent_experiment == NULL
			|| this->root_experiment->root_state == ROOT_EXPERIMENT_STATE_EXPERIMENT) {
		if (run_helper.experiment_histories.back() == history) {
			history->instance_count++;

			bool is_target = false;
			if (!history->has_target) {
				double target_probability;
				if (history->instance_count > this->average_instances_per_run) {
					target_probability = 0.5;
				} else {
					target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - history->instance_count));
				}
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < target_probability) {
					is_target = true;
				}
			}

			if (is_target) {
				history->has_target = true;

				context[context.size() - this->scope_context.size()].scope_history->experiment_history = history;

				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					history->experiment_index.push_back(context[context.size() - this->scope_context.size() + c_index].scope_history->node_histories.size());
				}
			}
		}
	}

	if (this->best_step_types.size() == 0) {
		if (this->exit_node != NULL) {
			curr_node = this->exit_node;
		} else {
			curr_node = this->best_exit_next_node;
		}
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			curr_node = this->best_actions[0];
		} else {
			curr_node = this->best_scopes[0];
		}
	}
}

void pass_through_inner_create_experiment_helper(
		vector<Scope*>& scope_context,
		vector<AbstractNode*>& node_context,
		vector<vector<Scope*>>& possible_scope_contexts,
		vector<vector<AbstractNode*>>& possible_node_contexts,
		vector<bool>& possible_is_branch,
		ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;

			node_context.back() = action_node;

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(false);

			node_context.back() = NULL;
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			uniform_int_distribution<int> inner_distribution(0, 1);
			if (inner_distribution(generator) == 0) {
				pass_through_inner_create_experiment_helper(
					scope_context,
					node_context,
					possible_scope_contexts,
					possible_node_contexts,
					possible_is_branch,
					scope_node_history->scope_history);
			}

			if (scope_node_history->normal_exit) {
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_is_branch.push_back(false);
			}

			node_context.back() = NULL;
		} else {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;

			node_context.back() = node_history->node;

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(branch_node_history->is_branch);

			node_context.back() = NULL;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void pass_through_create_experiment_helper(vector<int>& experiment_index,
										   vector<Scope*>& scope_context,
										   vector<AbstractNode*>& node_context,
										   vector<vector<Scope*>>& possible_scope_contexts,
										   vector<vector<AbstractNode*>>& possible_node_contexts,
										   vector<bool>& possible_is_branch,
										   ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	if (experiment_index.size() > 1) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[experiment_index[0]-1];

		node_context.back() = scope_node_history->node;

		vector<int> inner_experiment_index(experiment_index.begin()+1, experiment_index.end());
		pass_through_create_experiment_helper(inner_experiment_index,
											  scope_context,
											  node_context,
											  possible_scope_contexts,
											  possible_node_contexts,
											  possible_is_branch,
											  scope_node_history->scope_history);

		if (scope_node_history->normal_exit) {
			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(false);
		}

		node_context.back() = NULL;
	}

	for (int h_index = experiment_index[0]; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			if (h_index == 0 || action_node->action.move != ACTION_NOOP) {
				node_context.back() = action_node;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_is_branch.push_back(false);

				node_context.back() = NULL;
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			uniform_int_distribution<int> inner_distribution(0, 1);
			if (inner_distribution(generator) == 0) {
				pass_through_inner_create_experiment_helper(
					scope_context,
					node_context,
					possible_scope_contexts,
					possible_node_contexts,
					possible_is_branch,
					scope_node_history->scope_history);
			}

			if (scope_node_history->normal_exit) {
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_is_branch.push_back(false);
			}

			node_context.back() = NULL;
		} else {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;

			node_context.back() = node_history->node;

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(branch_node_history->is_branch);

			node_context.back() = NULL;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void PassThroughExperiment::experiment_backprop(
		double target_val,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	if (history->has_target
			&& !run_helper.exceeded_limit
			&& history->experiments_seen_order.size() == 0) {
		vector<vector<Scope*>> possible_scope_contexts;
		vector<vector<AbstractNode*>> possible_node_contexts;
		vector<bool> possible_is_branch;

		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		pass_through_create_experiment_helper(history->experiment_index,
											  scope_context,
											  node_context,
											  possible_scope_contexts,
											  possible_node_contexts,
											  possible_is_branch,
											  history->scope_history);

		if (possible_scope_contexts.size() > 0) {
			uniform_int_distribution<int> possible_distribution(0, (int)possible_scope_contexts.size()-1);
			int rand_index = possible_distribution(generator);

			uniform_int_distribution<int> experiment_type_distribution(0, 1);
			if (experiment_type_distribution(generator) == 0) {
				BranchExperiment* new_experiment = new BranchExperiment(
					possible_scope_contexts[rand_index],
					possible_node_contexts[rand_index],
					possible_is_branch[rand_index],
					this,
					false);

				/**
				 * - insert at front to match finalize order
				 */
				possible_node_contexts[rand_index].back()->experiments.insert(possible_node_contexts[rand_index].back()->experiments.begin(), new_experiment);
			} else {
				PassThroughExperiment* new_experiment = new PassThroughExperiment(
					possible_scope_contexts[rand_index],
					possible_node_contexts[rand_index],
					possible_is_branch[rand_index],
					this);

				/**
				 * - insert at front to match finalize order
				 */
				possible_node_contexts[rand_index].back()->experiments.insert(possible_node_contexts[rand_index].back()->experiments.begin(), new_experiment);
			}
		}
	}
}
