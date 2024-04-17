#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_SAMPLES_PER_ITER = 2;
#else
const int NUM_SAMPLES_PER_ITER = 40;
#endif /* MDEBUG */

void PassThroughExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
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

void PassThroughExperiment::explore_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->curr_score += target_val - this->existing_average_score;

	this->sub_state_iter++;
	if (this->sub_state_iter >= NUM_SAMPLES_PER_ITER) {
		this->curr_score /= NUM_SAMPLES_PER_ITER;
		#if defined(MDEBUG) && MDEBUG
		/**
		 * - at least has a chance to not exceed limit
		 */
		if (!run_helper.exceeded_limit) {
		#else
		if (this->curr_score > this->best_score) {
		#endif /* MDEBUG */
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else {
					if (s_index == 0) {
						map<int, Scope*>::iterator it = solution->scopes.find(this->best_scopes[s_index]->scope->id);
						if (it == solution->scopes.end()) {
							delete this->best_scopes[s_index]->scope;
						}
					}

					delete this->best_scopes[s_index];
				}
			}

			this->best_score = curr_score;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_scopes = this->curr_scopes;
			this->best_exit_depth = this->curr_exit_depth;
			this->best_exit_next_node = this->curr_exit_next_node;

			this->curr_score = 0.0;
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_scopes.clear();
		} else {
			for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
				if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->curr_actions[s_index];
				} else {
					if (s_index == 0) {
						map<int, Scope*>::iterator it = solution->scopes.find(this->curr_scopes[s_index]->scope->id);
						if (it == solution->scopes.end()) {
							delete this->curr_scopes[s_index]->scope;
						}
					}

					delete this->curr_scopes[s_index];
				}
			}

			this->curr_score = 0.0;
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_scopes.clear();
		}

		this->state_iter++;
		if (this->state_iter >= PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (this->best_score != numeric_limits<double>::lowest()) {
			#else
			if (this->best_score >= 0.0) {
			#endif /* MDEBUG */
				// cout << "PassThrough" << endl;
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
				// for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				// 	if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				// 		cout << " " << this->best_actions[s_index]->action.move;
				// 	} else {
				// 		cout << " E";
				// 	}
				// }
				// cout << endl;

				// cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
				// if (this->best_exit_next_node == NULL) {
				// 	cout << "this->best_exit_next_node->id: " << -1 << endl;
				// } else {
				// 	cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				// }

				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->parent = this->scope_context.back();
						this->best_actions[s_index]->id = this->scope_context.back()->node_counter;
						this->scope_context.back()->node_counter++;
					} else {
						this->best_scopes[s_index]->parent = this->scope_context.back();
						this->best_scopes[s_index]->id = this->scope_context.back()->node_counter;
						this->scope_context.back()->node_counter++;

						if (this->best_scopes[s_index]->scope->id == -1) {
							this->best_scopes[s_index]->scope->id = solution->scope_counter;
							solution->scope_counter++;
						}
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
					} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
						this->best_scopes[s_index]->next_node_id = next_node_id;
						this->best_scopes[s_index]->next_node = next_node;
					}
				}

				this->o_target_val_histories.reserve(NUM_DATAPOINTS);

				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		} else {
			uniform_int_distribution<int> in_place_distribution(0, 1);
			// if (in_place_distribution(generator) == 0) {
			if (false) {
				this->curr_exit_depth = 0;
				switch (this->node_context.back()->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)this->node_context.back();
						this->curr_exit_next_node = action_node->next_node;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
						this->curr_exit_next_node = scope_node->next_node;
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)this->node_context.back();
						if (this->is_branch) {
							this->curr_exit_next_node = branch_node->branch_next_node;
						} else {
							this->curr_exit_next_node = branch_node->original_next_node;
						}
					}
					break;
				}

				Scope* new_scope = new Scope();
				new_scope->id = -1;

				ActionNode* starting_noop_node = new ActionNode();
				starting_noop_node->parent = new_scope;
				starting_noop_node->id = 0;
				starting_noop_node->action = Action(ACTION_NOOP);
				new_scope->nodes[0] = starting_noop_node;
				new_scope->node_counter = 1;

				uniform_int_distribution<int> uniform_distribution(0, 1);
				geometric_distribution<int> geometric_distribution(0.5);
				int new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);

				uniform_int_distribution<int> default_distribution(0, 3);
				for (int s_index = 0; s_index < new_num_steps; s_index++) {
					bool default_to_action = true;
					if (default_distribution(generator) != 0) {
						ScopeNode* new_scope_node = create_existing();
						if (new_scope_node != NULL) {
							new_scope_node->parent = new_scope;
							new_scope_node->id = new_scope->node_counter;
							new_scope->node_counter++;
							new_scope->nodes[new_scope_node->id] = new_scope_node;

							default_to_action = false;
						}
					}

					if (default_to_action) {
						ActionNode* new_action_node = new ActionNode();
						new_action_node->action = problem_type->random_action();

						new_action_node->parent = new_scope;
						new_action_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_action_node->id] = new_action_node;
					}
				}

				ActionNode* ending_noop_node = new ActionNode();
				ending_noop_node->parent = new_scope;
				ending_noop_node->id = new_scope->node_counter;
				new_scope->node_counter++;
				ending_noop_node->action = Action(ACTION_NOOP);
				ending_noop_node->next_node_id = -1;
				ending_noop_node->next_node = NULL;
				new_scope->nodes[ending_noop_node->id] = ending_noop_node;

				for (int n_index = 0; n_index < (int)new_scope->nodes.size()-1; n_index++) {
					if (new_scope->nodes[n_index]->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)new_scope->nodes[n_index];
						action_node->next_node_id = n_index+1;
						action_node->next_node = new_scope->nodes[n_index+1];
					} else {
						ScopeNode* scope_node = (ScopeNode*)new_scope->nodes[n_index];
						scope_node->next_node_id = n_index+1;
						scope_node->next_node = new_scope->nodes[n_index+1];
					}
				}

				{
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->scope = new_scope;

					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(NULL);

					this->curr_scopes.push_back(new_scope_node);
				}
				{
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->scope = new_scope;

					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(NULL);

					this->curr_scopes.push_back(new_scope_node);
				}
				{
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->scope = new_scope;

					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(NULL);

					this->curr_scopes.push_back(new_scope_node);
				}
			} else {
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
				for (int s_index = 0; s_index < new_num_steps; s_index++) {
					bool default_to_action = true;
					if (default_distribution(generator) != 0) {
						ScopeNode* new_scope_node = create_existing();
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
			}

			this->sub_state_iter = 0;
		}
	}
}
