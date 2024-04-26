#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
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

				context.back().scope_history->experiment_history = history;

				history->experiment_index = context.back().scope_history->node_histories.size();
			}
		}
	}

	if (this->is_pass_through) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}

		return true;
	} else {
		run_helper.num_decisions++;

		vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			ScopeHistory* curr_scope_history = context.back().scope_history;
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
				curr_node = this->best_exit_next_node;
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

void BranchExperiment::experiment_backprop(double target_val,
										   RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	if (history->has_target
			&& !run_helper.exceeded_limit
			&& history->experiments_seen_order.size() == 0) {
		vector<AbstractNode*> possible_node_contexts;
		vector<bool> possible_is_branch;

		for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = history->scope_history->node_histories.begin();
				it != history->scope_history->node_histories.end(); it++) {
			if (it->second->index >= history->experiment_index) {
				switch (it->first->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)it->first;

						if (action_node->action.move == ACTION_NOOP) {
							map<int, AbstractNode*>::iterator it = action_node->parent->nodes.find(action_node->id);
							if (it == action_node->parent->nodes.end()) {
								/**
								 * - new ending node edge case
								 */
								continue;
							}
						}

						possible_node_contexts.push_back(it->first);
						possible_is_branch.push_back(false);
					}

					break;
				case NODE_TYPE_SCOPE:
					{
						possible_node_contexts.push_back(it->first);
						possible_is_branch.push_back(false);
					}

					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;

						possible_node_contexts.push_back(it->first);
						possible_is_branch.push_back(branch_node_history->is_branch);
					}

					break;
				}
			}
		}

		uniform_int_distribution<int> possible_distribution(0, (int)possible_node_contexts.size()-1);
		int rand_index = possible_distribution(generator);

		uniform_int_distribution<int> experiment_type_distribution(0, 1);
		if (experiment_type_distribution(generator) == 0) {
			BranchExperiment* new_experiment = new BranchExperiment(
				this->scope_context,
				possible_node_contexts[rand_index],
				possible_is_branch[rand_index],
				this);

			/**
			 * - insert at front to match finalize order
			 */
			possible_node_contexts[rand_index]->experiments.insert(possible_node_contexts[rand_index]->experiments.begin(), new_experiment);
		} else {
			PassThroughExperiment* new_experiment = new PassThroughExperiment(
				this->scope_context,
				possible_node_contexts[rand_index],
				possible_is_branch[rand_index],
				this);

			/**
			 * - insert at front to match finalize order
			 */
			possible_node_contexts[rand_index]->experiments.insert(possible_node_contexts[rand_index]->experiments.begin(), new_experiment);
		}
	}
}
