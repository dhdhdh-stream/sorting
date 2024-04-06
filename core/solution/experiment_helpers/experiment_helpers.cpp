#include "experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void Experiment::experiment_activate(AbstractNode*& curr_node,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper,
									 ExperimentHistory* history) {
	if (this->is_pass_through) {
		if (this->throw_id != -1) {
			run_helper.throw_id = -1;
		}

		if (this->step_types.size() == 0) {
			if (this->exit_node != NULL) {
				curr_node = this->exit_node;
			} else {
				curr_node = this->exit_next_node;
			}
		} else {
			if (this->step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->actions[0];
			} else {
				curr_node = this->scopes[0];
			}
		}
	} else {
		vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
				action_node->hook_indexes.push_back(i_index);
				action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
				action_node->hook_obs_indexes.push_back(this->input_obs_indexes[i_index]);
			} else {
				BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
				branch_node->hook_indexes.push_back(i_index);
				branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
			}
		}
		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		input_vals_helper(0,
						  this->input_max_depth,
						  scope_context,
						  node_context,
						  input_vals,
						  context[context.size() - this->scope_context.size()].scope_history);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
				action_node->hook_indexes.clear();
				action_node->hook_scope_contexts.clear();
				action_node->hook_node_contexts.clear();
				action_node->hook_obs_indexes.clear();
			} else {
				BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
				branch_node->hook_indexes.clear();
				branch_node->hook_scope_contexts.clear();
				branch_node->hook_node_contexts.clear();
			}
		}

		double existing_predicted_score = this->existing_average_score;
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			existing_predicted_score += input_vals[i_index] * this->existing_linear_weights[i_index];
		}
		if (this->existing_network != NULL) {
			vector<vector<double>> existing_network_input_vals(this->existing_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
				existing_network_input_vals[i_index] = vector<double>(this->existing_network_input_indexes[i_index].size());
				for (int s_index = 0; s_index < (int)this->existing_network_input_indexes[i_index].size(); s_index++) {
					existing_network_input_vals[i_index][s_index] = input_vals[this->existing_network_input_indexes[i_index][s_index]];
				}
			}
			this->existing_network->activate(existing_network_input_vals);
			existing_predicted_score += this->existing_network->output->acti_vals[0];
		}

		double new_predicted_score = this->new_average_score;
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			new_predicted_score += input_vals[i_index] * this->new_linear_weights[i_index];
		}
		if (this->new_network != NULL) {
			vector<vector<double>> new_network_input_vals(this->new_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
				new_network_input_vals[i_index] = vector<double>(this->new_network_input_indexes[i_index].size());
				for (int s_index = 0; s_index < (int)this->new_network_input_indexes[i_index].size(); s_index++) {
					new_network_input_vals[i_index][s_index] = input_vals[this->new_network_input_indexes[i_index][s_index]];
				}
			}
			this->new_network->activate(new_network_input_vals);
			new_predicted_score += this->new_network->output->acti_vals[0];
		}

		#if defined(MDEBUG) && MDEBUG
		bool decision_is_branch;
		if (run_helper.curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		bool decision_is_branch = new_predicted_score >= existing_predicted_score;
		#endif /* MDEBUG */

		/**
		 * - don't include branch_node history
		 *   - can cause issues when chaining experiments
		 *     - TODO: find good way to handle
		 */
		if (decision_is_branch) {
			if (this->throw_id != -1) {
				run_helper.throw_id = -1;
			}

			if (this->step_types.size() == 0) {
				if (this->exit_node != NULL) {
					curr_node = this->exit_node;
				} else {
					curr_node = this->exit_next_node;
				}
			} else {
				if (this->step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->actions[0];
				} else {
					curr_node = this->scopes[0];
				}
			}
		}
	}

	if (this->parent_experiment == NULL
			|| this->root_experiment->state == EXPERIMENT_STATE_EXPERIMENT) {
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

			if (scope_node_history->normal_exit
					|| scope_node_history->throw_id != -1) {
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_is_branch.push_back(false);
				possible_throw_id.push_back(scope_node_history->throw_id);
			}

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

			if (scope_node_history->normal_exit
					|| scope_node_history->throw_id != -1) {
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_is_branch.push_back(false);
				possible_throw_id.push_back(scope_node_history->throw_id);
			}

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

void Experiment::experiment_backprop(double target_val,
									 RunHelper& run_helper) {
	ExperimentHistory* history = run_helper.experiment_histories.back();

	if (history->has_target
			&& !run_helper.exceeded_limit
			&& history->experiments_seen_order.size() == 0) {
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

		if (possible_scope_contexts.size() > 0) {
			uniform_int_distribution<int> possible_distribution(0, (int)possible_scope_contexts.size()-1);
			int rand_index = possible_distribution(generator);

			Experiment* new_experiment = new Experiment(
				possible_scope_contexts[rand_index],
				possible_node_contexts[rand_index],
				possible_is_branch[rand_index],
				possible_throw_id[rand_index],
				this,
				false);

			/**
			 * - insert at front to match finalize order
			 */
			possible_node_contexts[rand_index].back()->experiments.insert(possible_node_contexts[rand_index].back()->experiments.begin(), new_experiment);
		}
	}
}