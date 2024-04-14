#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 5;
#else
const int EXPLORE_ITERS = 500;
#endif /* MDEBUG */

void BranchExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper,
		BranchExperimentHistory* history) {
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

		double predicted_score = this->existing_average_score;
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			predicted_score += input_vals[i_index] * this->existing_linear_weights[i_index];
		}
		if (this->existing_network != NULL) {
			vector<vector<double>> network_input_vals(this->existing_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
				network_input_vals[i_index] = vector<double>(this->existing_network_input_indexes[i_index].size());
				for (int s_index = 0; s_index < (int)this->existing_network_input_indexes[i_index].size(); s_index++) {
					network_input_vals[i_index][s_index] = input_vals[this->existing_network_input_indexes[i_index][s_index]];
				}
			}
			this->existing_network->activate(network_input_vals);
			predicted_score += this->existing_network->output->acti_vals[0];
		}

		history->existing_predicted_score = predicted_score;

		vector<pair<int,AbstractNode*>> possible_exits;

		if (this->node_context.back()->type == NODE_TYPE_ACTION
				&& ((ActionNode*)this->node_context.back())->next_node == NULL) {
			possible_exits.push_back({0, NULL});
		}

		gather_possible_exits(possible_exits,
							  this->scope_context,
							  this->node_context,
							  this->is_branch);

		if (possible_exits.size() == 0) {
			switch (this->node_context.back()->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context.back();
					possible_exits.push_back({0, action_node->next_node});
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
					possible_exits.push_back({0, scope_node->next_node});
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context.back();
					if (this->is_branch) {
						possible_exits.push_back({0, branch_node->branch_next_node});
					} else {
						possible_exits.push_back({0, branch_node->original_next_node});
					}
				}
				break;
			}
		}

		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
		int random_index = distribution(generator);
		this->curr_exit_depth = possible_exits[random_index].first;
		this->curr_exit_next_node = possible_exits[random_index].second;

		int new_num_steps;
		uniform_int_distribution<int> uniform_distribution(0, 1);
		geometric_distribution<int> geometric_distribution(0.5);
		if (random_index == 0) {
			new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
		} else {
			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
		}

		uniform_int_distribution<int> default_distribution(0, 3);
		uniform_int_distribution<int> curr_scope_distribution(0, 2);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool default_to_action = true;
			if (default_distribution(generator) != 0) {
				Scope* scope;
				if (solution->scopes[solution->curr_scope_id]->layer == 0
						|| curr_scope_distribution(generator) == 0) {
					scope = solution->scopes[solution->curr_scope_id];
				} else {
					uniform_int_distribution<int> child_distribution(0, MAX_NUM_CHILDREN-1);
					int scope_id = solution->scopes[solution->curr_scope_id]->child_ids[child_distribution(generator)];
					scope = solution->scopes[scope_id];
				}
				ScopeNode* new_scope_node = create_existing(scope,
															run_helper);
				if (new_scope_node != NULL) {
					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(NULL);

					this->curr_scopes.push_back(new_scope_node);

					default_to_action = false;
				}
			}

			if (default_to_action) {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->curr_actions.push_back(new_action_node);

				this->curr_scopes.push_back(NULL);
			}
		}

		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = new ActionNodeHistory(this->curr_actions[s_index]);
				this->curr_actions[s_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					action_node_history);
				delete action_node_history;
			} else {
				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->curr_scopes[s_index]);
				this->curr_scopes[s_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					scope_node_history);
				delete scope_node_history;
			}
		}

		if (this->curr_exit_depth == 0) {
			curr_node = this->curr_exit_next_node;
		} else {
			exit_depth = this->curr_exit_depth-1;
			exit_node = this->curr_exit_next_node;
		}
	}
}

void BranchExperiment::explore_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	if (history->has_target) {
		
		double curr_surprise = target_val - history->existing_predicted_score;

		bool select = false;
		if (this->explore_type == EXPLORE_TYPE_BEST) {
			#if defined(MDEBUG) && MDEBUG
			if (!run_helper.exceeded_limit) {
			#else
			if (curr_surprise > this->best_surprise) {
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_actions[s_index];
					} else {
						delete this->best_scopes[s_index];
					}
				}

				this->best_surprise = curr_surprise;
				this->best_step_types = this->curr_step_types;
				this->best_actions = this->curr_actions;
				this->best_scopes = this->curr_scopes;
				this->best_exit_depth = this->curr_exit_depth;
				this->best_exit_next_node = this->curr_exit_next_node;

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			} else {
				for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
					if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->curr_actions[s_index];
					} else {
						delete this->curr_scopes[s_index];
					}
				}

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			}

			if (this->state_iter == EXPLORE_ITERS-1
					&& this->best_surprise > 0.0) {
				select = true;
			}
		} else if (this->explore_type == EXPLORE_TYPE_NEUTRAL) {
			#if defined(MDEBUG) && MDEBUG
			if (!run_helper.exceeded_limit) {
			#else
			if (curr_surprise >= 0.0) {
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_actions[s_index];
					} else {
						delete this->best_scopes[s_index];
					}
				}

				this->best_surprise = curr_surprise;
				this->best_step_types = this->curr_step_types;
				this->best_actions = this->curr_actions;
				this->best_scopes = this->curr_scopes;
				this->best_exit_depth = this->curr_exit_depth;
				this->best_exit_next_node = this->curr_exit_next_node;

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();

				select = true;
			} else {
				for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
					if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->curr_actions[s_index];
					} else {
						delete this->curr_scopes[s_index];
					}
				}

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			}
		} else if (this->explore_type == EXPLORE_TYPE_GOOD) {
			#if defined(MDEBUG) && MDEBUG
			if (!run_helper.exceeded_limit) {
			#else
			if (curr_surprise >= this->existing_score_standard_deviation) {
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_actions[s_index];
					} else {
						delete this->best_scopes[s_index];
					}
				}

				this->best_surprise = curr_surprise;
				this->best_step_types = this->curr_step_types;
				this->best_actions = this->curr_actions;
				this->best_scopes = this->curr_scopes;
				this->best_exit_depth = this->curr_exit_depth;
				this->best_exit_next_node = this->curr_exit_next_node;

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();

				select = true;
			} else {
				for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
					if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->curr_actions[s_index];
					} else {
						delete this->curr_scopes[s_index];
					}
				}

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			}
		}

		if (select) {
			// cout << "curr_surprise: " << curr_surprise << endl;

			// cout << "this->scope_context:" << endl;
			// for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			// 	cout << c_index << ": " << this->scope_context[c_index]->id << endl;
			// }
			// cout << "this->node_context:" << endl;
			// for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
			// 	cout << c_index << ": " << this->node_context[c_index]->id << endl;
			// }
			// cout << "this->is_branch: " << this->is_branch << endl;
			// cout << "new explore path:";
			// for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
			// 	if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			// 		cout << " " << this->actions[s_index]->action.move;
			// 	} else {
			// 		cout << " E" << this->existing_scopes[s_index]->scope->id;
			// 	}
			// }
			// cout << endl;

			// cout << "this->exit_depth: " << this->exit_depth << endl;
			// if (this->exit_next_node == NULL) {
			// 	cout << "this->exit_next_node->id: " << -1 << endl;
			// } else {
			// 	cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
			// }
			// cout << endl;

			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					this->best_actions[s_index]->parent = this->scope_context.back();
					this->best_actions[s_index]->id = this->scope_context.back()->node_counter;
					this->scope_context.back()->node_counter++;
				} else {
					this->best_scopes[s_index]->parent = this->scope_context.back();
					this->best_scopes[s_index]->id = this->scope_context.back()->node_counter;
					this->scope_context.back()->node_counter++;
				}
			}

			int exit_node_id;
			AbstractNode* exit_node;
			if (this->best_exit_depth > 0) {
				ExitNode* new_exit_node = new ExitNode();
				new_exit_node->parent = this->scope_context.back();
				new_exit_node->id = this->scope_context.back()->node_counter;
				this->scope_context.back()->node_counter++;

				new_exit_node->exit_depth = this->best_exit_depth;
				new_exit_node->next_node_parent = this->scope_context[this->scope_context.size()-1 - this->best_exit_depth];
				new_exit_node->next_node_id = this->best_exit_next_node->id;
				new_exit_node->next_node = this->best_exit_next_node;

				this->exit_node = new_exit_node;

				exit_node_id = new_exit_node->id;
				exit_node = new_exit_node;
			} else {
				if (this->best_exit_next_node == NULL) {
					ActionNode* new_ending_node = new ActionNode();
					new_ending_node->parent = this->scope_context.back();
					new_ending_node->id = this->scope_context.back()->node_counter;
					this->scope_context.back()->node_counter++;

					new_ending_node->action = Action(ACTION_NOOP);

					new_ending_node->next_node_id = -1;
					new_ending_node->next_node = NULL;

					this->ending_node = new_ending_node;

					exit_node_id = new_ending_node->id;
					exit_node = new_ending_node;
				} else {
					exit_node_id = this->best_exit_next_node->id;
					exit_node = this->best_exit_next_node;
				}
			}

			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				int next_node_id;
				AbstractNode* next_node;
				if (s_index == (int)this->best_step_types.size()-1) {
					next_node_id = exit_node_id;
					next_node = exit_node;
				} else {
					if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
						next_node_id = this->best_actions[s_index+1]->id;
						next_node = this->best_actions[s_index+1];
					} else {
						next_node_id = this->best_scopes[s_index+1]->id;
						next_node = this->best_scopes[s_index+1];
					}
				}

				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					this->best_actions[s_index]->next_node_id = next_node_id;
					this->best_actions[s_index]->next_node = next_node;
				} else {
					this->best_scopes[s_index]->next_node_id = next_node_id;
					this->best_scopes[s_index]->next_node = next_node;
				}
			}

			this->i_scope_histories.reserve(NUM_DATAPOINTS);
			this->i_target_val_histories.reserve(NUM_DATAPOINTS);

			this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW;
			this->state_iter = 0;
		} else {
			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
