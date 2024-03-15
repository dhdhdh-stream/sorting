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

void PassThroughExperiment::experiment_activate(vector<int>& context_match_indexes,
												AbstractNode*& curr_node,
												vector<ContextLayer>& context,
												RunHelper& run_helper,
												PassThroughExperimentHistory* history) {
	if (this->parent_experiment == NULL
			|| this->root_experiment->state == PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT) {
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

				context[context_match_indexes[0]].scope_history->pass_through_experiment_history = history;

				for (int c_index = context_match_indexes[0]; c_index < (int)context.size(); c_index++) {
					history->experiment_index.push_back(context[c_index].scope_history->node_histories.size());
				}
			}
		}
	}

	if (this->throw_id != -1) {
		run_helper.throw_id = -1;
	}

	if (this->best_step_types.size() == 0) {
		curr_node = this->exit_node;
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			curr_node = this->best_actions[0];
		} else if (this->best_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
			curr_node = this->best_existing_scopes[0];
		} else {
			curr_node = this->best_potential_scopes[0];
		}
	}
}

void inner_create_experiment_helper(vector<Scope*>& scope_context,
									vector<AbstractNode*>& node_context,
									vector<vector<Scope*>>& possible_scope_contexts,
									vector<vector<AbstractNode*>>& possible_node_contexts,
									vector<bool>& possible_is_branch,
									vector<int>& possible_throw_id,
									ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			if (h_index == 0 || action_node->action.move != ACTION_NOOP) {
				node_context.back() = action_node;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_is_branch.push_back(false);
				possible_throw_id.push_back(-1);

				node_context.back() = NULL;
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			uniform_int_distribution<int> inner_distribution(0, 1);
			if (inner_distribution(generator) == 0) {
				inner_create_experiment_helper(scope_context,
											   node_context,
											   possible_scope_contexts,
											   possible_node_contexts,
											   possible_is_branch,
											   possible_throw_id,
											   scope_node_history->scope_history);
			}

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(false);
			possible_throw_id.push_back(scope_node_history->throw_id);

			node_context.back() = NULL;
		} else {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;

			node_context.back() = node_history->node;

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(branch_node_history->is_branch);
			possible_throw_id.push_back(-1);

			node_context.back() = NULL;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void create_experiment_helper(vector<int>& experiment_index,
							  vector<Scope*>& scope_context,
							  vector<AbstractNode*>& node_context,
							  vector<vector<Scope*>>& possible_scope_contexts,
							  vector<vector<AbstractNode*>>& possible_node_contexts,
							  vector<bool>& possible_is_branch,
							  vector<int>& possible_throw_id,
							  ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	if (experiment_index.size() > 1) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[experiment_index[0]-1];

		node_context.back() = scope_node_history->node;

		vector<int> inner_experiment_index(experiment_index.begin()+1, experiment_index.end());
		create_experiment_helper(inner_experiment_index,
								 scope_context,
								 node_context,
								 possible_scope_contexts,
								 possible_node_contexts,
								 possible_is_branch,
								 possible_throw_id,
								 scope_node_history->scope_history);

		possible_scope_contexts.push_back(scope_context);
		possible_node_contexts.push_back(node_context);
		possible_is_branch.push_back(false);
		possible_throw_id.push_back(scope_node_history->throw_id);

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
				possible_throw_id.push_back(-1);

				node_context.back() = NULL;
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			uniform_int_distribution<int> inner_distribution(0, 1);
			if (inner_distribution(generator) == 0) {
				inner_create_experiment_helper(scope_context,
											   node_context,
											   possible_scope_contexts,
											   possible_node_contexts,
											   possible_is_branch,
											   possible_throw_id,
											   scope_node_history->scope_history);
			}

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(false);
			possible_throw_id.push_back(scope_node_history->throw_id);

			node_context.back() = NULL;
		} else {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;

			node_context.back() = node_history->node;

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(branch_node_history->is_branch);
			possible_throw_id.push_back(-1);

			node_context.back() = NULL;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void PassThroughExperiment::experiment_backprop(
		double target_val,
		RunHelper& run_helper,
		PassThroughExperimentHistory* history) {
	if (history->has_target) {
		vector<vector<Scope*>> possible_scope_contexts;
		vector<vector<AbstractNode*>> possible_node_contexts;
		vector<bool> possible_is_branch;
		vector<int> possible_throw_id;

		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		create_experiment_helper(history->experiment_index,
								 scope_context,
								 node_context,
								 possible_scope_contexts,
								 possible_node_contexts,
								 possible_is_branch,
								 possible_throw_id,
								 history->scope_history);

		uniform_int_distribution<int> possible_distribution(0, (int)possible_scope_contexts.size()-1);
		int rand_index = possible_distribution(generator);

		vector<Scope*> new_scope_context;
		vector<AbstractNode*> new_node_context;
		bool new_is_fuzzy_match;

		uniform_int_distribution<int> is_strict_distribution(0, 2);
		if (is_strict_distribution(generator) == 0) {
			uniform_int_distribution<int> stop_distribution(0, 2);
			int context_size = 1;
			while (true) {
				if (context_size < (int)possible_scope_contexts[rand_index].size() && !stop_distribution(generator) == 0) {
					context_size++;
				} else {
					break;
				}
			}
			/**
			 * - minimize context to generalize/maximize impact
			 */

			new_scope_context = vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end());
			new_node_context = vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end());
			new_is_fuzzy_match = false;
		} else {
			geometric_distribution<int> num_layers_distribution(0.33);
			int num_layers = num_layers_distribution(generator);
			if (num_layers > (int)possible_scope_contexts[rand_index].size()-1) {
				num_layers = possible_scope_contexts[rand_index].size()-1;
			}

			vector<bool> layer_included(possible_scope_contexts[rand_index].size()-1, false);

			vector<int> remaining_indexes(possible_scope_contexts[rand_index].size()-1);
			for (int l_index = 0; l_index < (int)possible_scope_contexts[rand_index].size()-1; l_index++) {
				remaining_indexes[l_index] = l_index;
			}

			for (int l_index = 0; l_index < num_layers; l_index++) {
				uniform_int_distribution<int> index_distribution(0, remaining_indexes.size()-1);
				int index = index_distribution(generator);

				layer_included[remaining_indexes[index]] = true;

				remaining_indexes.erase(remaining_indexes.begin() + index);
			}

			for (int l_index = 0; l_index < (int)possible_scope_contexts[rand_index].size()-1; l_index++) {
				if (layer_included[l_index]) {
					new_scope_context.push_back(possible_scope_contexts[rand_index][l_index]);
					new_node_context.push_back(possible_node_contexts[rand_index][l_index]);
				}
			}
			new_scope_context.push_back(possible_scope_contexts[rand_index].back());
			new_node_context.push_back(possible_node_contexts[rand_index].back());

			new_is_fuzzy_match = true;
		}

		if (possible_node_contexts[rand_index].back()->type == NODE_TYPE_ACTION) {
			uniform_int_distribution<int> pass_through_distribution(0, 1);
			if (pass_through_distribution(generator) == 0) {
				PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
					new_scope_context,
					new_node_context,
					new_is_fuzzy_match,
					false,
					-1,
					this);

				ActionNode* action_node = (ActionNode*)possible_node_contexts[rand_index].back();
				action_node->experiments.push_back(new_pass_through_experiment);
			} else {
				BranchExperiment* new_branch_experiment = new BranchExperiment(
					new_scope_context,
					new_node_context,
					new_is_fuzzy_match,
					false,
					-1,
					this,
					false);

				ActionNode* action_node = (ActionNode*)possible_node_contexts[rand_index].back();
				action_node->experiments.push_back(new_branch_experiment);
			}
		} else if (possible_node_contexts[rand_index].back()->type == NODE_TYPE_SCOPE) {
			uniform_int_distribution<int> pass_through_distribution(0, 1);
			if (pass_through_distribution(generator) == 0) {
				PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
					new_scope_context,
					new_node_context,
					new_is_fuzzy_match,
					false,
					possible_throw_id[rand_index],
					this);

				ScopeNode* scope_node = (ScopeNode*)possible_node_contexts[rand_index].back();
				scope_node->experiments.push_back(new_pass_through_experiment);
			} else {
				BranchExperiment* new_branch_experiment = new BranchExperiment(
					new_scope_context,
					new_node_context,
					new_is_fuzzy_match,
					false,
					possible_throw_id[rand_index],
					this,
					false);

				ScopeNode* scope_node = (ScopeNode*)possible_node_contexts[rand_index].back();
				scope_node->experiments.push_back(new_branch_experiment);
			}
		} else {
			uniform_int_distribution<int> pass_through_distribution(0, 1);
			if (pass_through_distribution(generator) == 0) {
				PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
					new_scope_context,
					new_node_context,
					new_is_fuzzy_match,
					possible_is_branch[rand_index],
					-1,
					this);

				BranchNode* branch_node = (BranchNode*)possible_node_contexts[rand_index].back();
				branch_node->experiments.push_back(new_pass_through_experiment);
				if (possible_is_branch[rand_index]) {
					branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_BRANCH);
				} else {
					branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_ORIGINAL);
				}
			} else {
				BranchExperiment* new_branch_experiment = new BranchExperiment(
					new_scope_context,
					new_node_context,
					new_is_fuzzy_match,
					possible_is_branch[rand_index],
					-1,
					this,
					false);

				BranchNode* branch_node = (BranchNode*)possible_node_contexts[rand_index].back();
				branch_node->experiments.push_back(new_branch_experiment);
				if (possible_is_branch[rand_index]) {
					branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_BRANCH);
				} else {
					branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_ORIGINAL);
				}
			}
		}
	}
}
