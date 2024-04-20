#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::experiment_activate(AbstractNode*& curr_node,
										   vector<ContextLayer>& context,
										   RunHelper& run_helper,
										   BranchExperimentHistory* history) {
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

	if (this->is_pass_through) {
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

		return true;
	} else {
		vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			ScopeHistory* curr_scope_history = context[context.size() - this->scope_context.size()].scope_history;
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					this->input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)this->input_scope_contexts[i_index].size()-1) {
						if (it->first->type == NODE_TYPE_ACTION) {
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
						} else {
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							if (branch_node_history->is_branch) {
								input_vals[i_index] = 1.0;
							} else {
								input_vals[i_index] = -1.0;
							}
						}
						break;
					} else {
						curr_layer++;
						curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
					}
				}
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

			return true;
		} else {
			return false;
		}
	}
}

void branch_inner_create_experiment_helper(vector<Scope*>& scope_context,
										   vector<AbstractNode*>& node_context,
										   vector<vector<Scope*>>& possible_scope_contexts,
										   vector<vector<AbstractNode*>>& possible_node_contexts,
										   vector<bool>& possible_is_branch,
										   ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->first->type) {
		case NODE_TYPE_ACTION:
			{
				node_context.back() = it->first;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_is_branch.push_back(false);

				node_context.back() = NULL;
			}

			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				node_context.back() = it->first;

				uniform_int_distribution<int> inner_distribution(0, 1);
				if (inner_distribution(generator) == 0) {
					branch_inner_create_experiment_helper(
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
			}

			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;

				node_context.back() = it->first;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_is_branch.push_back(branch_node_history->is_branch);

				node_context.back() = NULL;
			}

			break;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void BranchExperiment::branch_create_experiment_helper(
		vector<int>& experiment_index,
		vector<Scope*>& scope_context,
		vector<AbstractNode*>& node_context,
		vector<vector<Scope*>>& possible_scope_contexts,
		vector<vector<AbstractNode*>>& possible_node_contexts,
		vector<bool>& possible_is_branch,
		ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	if (experiment_index.size() > 1) {
		AbstractNode* ancestor_scope_node = this->node_context[
			this->scope_context.size() - experiment_index.size()];
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[ancestor_scope_node];

		node_context.back() = ancestor_scope_node;

		vector<int> inner_experiment_index(experiment_index.begin()+1, experiment_index.end());
		branch_create_experiment_helper(inner_experiment_index,
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

	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->index >= experiment_index[0]) {
			switch (it->first->type) {
			case NODE_TYPE_ACTION:
				{
					node_context.back() = it->first;

					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);
					possible_is_branch.push_back(false);

					node_context.back() = NULL;
				}

				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

					node_context.back() = it->first;

					// uniform_int_distribution<int> inner_distribution(0, 1);
					// if (inner_distribution(generator) == 0) {
					// 	pass_through_inner_create_experiment_helper(
					// 		scope_context,
					// 		node_context,
					// 		possible_scope_contexts,
					// 		possible_node_contexts,
					// 		possible_is_branch,
					// 		scope_node_history->scope_history);
					// }

					if (scope_node_history->normal_exit) {
						possible_scope_contexts.push_back(scope_context);
						possible_node_contexts.push_back(node_context);
						possible_is_branch.push_back(false);
					}

					node_context.back() = NULL;
				}

				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;

					node_context.back() = it->first;

					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);
					possible_is_branch.push_back(branch_node_history->is_branch);

					node_context.back() = NULL;
				}

				break;
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void BranchExperiment::experiment_backprop(double target_val,
										   RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	if (history->has_target
			&& !run_helper.exceeded_limit
			&& history->experiments_seen_order.size() == 0) {
		vector<vector<Scope*>> possible_scope_contexts;
		vector<vector<AbstractNode*>> possible_node_contexts;
		vector<bool> possible_is_branch;

		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		branch_create_experiment_helper(history->experiment_index,
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
