#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 2;
#else
const int EXPLORE_ITERS = 200;
#endif /* MDEBUG */

void BranchExperiment::explore_activate(AbstractNode*& curr_node,
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

		explore_target_activate(curr_node,
								problem,
								context,
								exit_depth,
								exit_node,
								run_helper,
								history);
	}
}

void BranchExperiment::explore_target_activate(AbstractNode*& curr_node,
											   Problem* problem,
											   vector<ContextLayer>& context,
											   int& exit_depth,
											   AbstractNode*& exit_node,
											   RunHelper& run_helper,
											   BranchExperimentHistory* history) {
	vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
			action_node->hook_indexes.push_back(i_index);
			action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
			action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
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

	bool is_repeat = false;
	uniform_int_distribution<int> repeat_distribution(0, 3);
	if (this->throw_id == -1
			&& this->parent_experiment == NULL
			&& repeat_distribution(generator)) {
		is_repeat = create_repeat(context,
								  (int)this->scope_context.size(),
								  this->curr_step_types,
								  this->curr_actions,
								  this->curr_existing_scopes,
								  this->curr_potential_scopes,
								  this->curr_catch_throw_ids);
		if (is_repeat) {
			this->curr_exit_depth = 0;
			this->curr_exit_next_node = curr_node;
			this->curr_exit_throw_id = -1;
		}
	}

	if (!is_repeat) {
		// exit
		vector<pair<int,AbstractNode*>> possible_exits;
		gather_possible_exits(possible_exits,
							  this->scope_context,
							  this->node_context,
							  this->is_branch,
							  this->throw_id,
							  run_helper);

		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
		int random_index = distribution(generator);
		this->curr_exit_depth = possible_exits[random_index].first;
		this->curr_exit_next_node = possible_exits[random_index].second;

		uniform_int_distribution<int> throw_distribution(0, 3);
		if (throw_distribution(generator) == 0) {
			uniform_int_distribution<int> reuse_existing_throw_distribution(0, 1);
			if (reuse_existing_throw_distribution(generator) == 0) {
				/**
				 * - simply allow duplicates
				 */
				vector<int> possible_throw_ids;
				for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
					ScopeNode* scope_node = (ScopeNode*)context[c_index].node;
					for (map<int, AbstractNode*>::iterator it = scope_node->catches.begin();
							it != scope_node->catches.end(); it++) {
						possible_throw_ids.push_back(it->first);
					}
				}

				if (possible_throw_ids.size() > 0) {
					uniform_int_distribution<int> possible_distribution(0, possible_throw_ids.size()-1);
					this->curr_exit_throw_id = possible_throw_ids[possible_distribution(generator)];
				} else {
					this->curr_exit_throw_id = TEMP_THROW_ID;
				}
			} else {
				this->curr_exit_throw_id = TEMP_THROW_ID;
			}
		} else {
			this->curr_exit_throw_id = -1;
		}

		// new path
		int new_num_steps;
		uniform_int_distribution<int> uniform_distribution(0, 1);
		geometric_distribution<int> geometric_distribution(0.5);
		if (this->curr_exit_depth == 0
				&& this->curr_exit_next_node == curr_node) {
			new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
		} else {
			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
		}

		uniform_int_distribution<int> type_distribution(0, 2);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool default_to_action = true;
			int type = type_distribution(generator);
			if (type == 0) {
				uniform_int_distribution<int> random_scope_distribution(0, 3);
				if (random_scope_distribution(generator) == 0) {
					uniform_int_distribution<int> distribution(0, solution->scopes.size()-1);
					Scope* scope = next(solution->scopes.begin(), distribution(generator))->second;
					default_to_action = !create_path(scope,
													 run_helper,
													 this->curr_step_types,
													 this->curr_actions,
													 this->curr_existing_scopes,
													 this->curr_potential_scopes,
													 this->curr_catch_throw_ids);
				} else {
					default_to_action = !create_path(context[context.size() - this->scope_context.size()].scope,
													 run_helper,
													 this->curr_step_types,
													 this->curr_actions,
													 this->curr_existing_scopes,
													 this->curr_potential_scopes,
													 this->curr_catch_throw_ids);
				}
			} else if (type == 1) {
				uniform_int_distribution<int> distribution(0, solution->scopes.size()-1);
				Scope* scope = next(solution->scopes.begin(), distribution(generator))->second;
				ScopeNode* new_scope_node = create_existing(scope,
															run_helper);
				if (new_scope_node != NULL) {
					this->curr_step_types.push_back(STEP_TYPE_EXISTING_SCOPE);
					this->curr_actions.push_back(NULL);

					this->curr_existing_scopes.push_back(new_scope_node);

					this->curr_potential_scopes.push_back(NULL);
					this->curr_catch_throw_ids.push_back(set<int>());

					default_to_action = false;
				}
			}

			if (default_to_action) {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->curr_actions.push_back(new_action_node);

				this->curr_existing_scopes.push_back(NULL);
				this->curr_potential_scopes.push_back(NULL);
				this->curr_catch_throw_ids.push_back(set<int>());
			}
		}
	}

	if (this->throw_id != -1) {
		run_helper.throw_id = -1;
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
		} else if (this->curr_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->curr_existing_scopes[s_index]);
			this->curr_existing_scopes[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->curr_potential_scopes[s_index]);
			this->curr_potential_scopes[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		}

		if (run_helper.throw_id != -1) {
			this->curr_catch_throw_ids[s_index].insert(run_helper.throw_id);
			run_helper.throw_id = -1;
		}
	}

	if (this->curr_exit_throw_id != -1) {
		run_helper.throw_id = this->curr_exit_throw_id;
	} else {
		if (this->curr_exit_depth == 0) {
			curr_node = this->curr_exit_next_node;
		} else {
			exit_depth = this->curr_exit_depth-1;
			exit_node = this->curr_exit_next_node;
		}
	}
}

void BranchExperiment::explore_backprop(double target_val,
										RunHelper& run_helper,
										BranchExperimentHistory* history) {
	if (history->has_target) {
		double curr_surprise = target_val - history->existing_predicted_score;
		#if defined(MDEBUG) && MDEBUG
		if (!run_helper.exceeded_limit) {
		#else
		if (curr_surprise > this->best_surprise) {
		#endif /* MDEBUG */
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
					delete this->best_existing_scopes[s_index];
				} else {
					delete this->best_potential_scopes[s_index]->scope;
					delete this->best_potential_scopes[s_index];
				}
			}

			this->best_surprise = curr_surprise;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_existing_scopes = this->curr_existing_scopes;
			this->best_potential_scopes = this->curr_potential_scopes;
			this->best_exit_depth = this->curr_exit_depth;
			this->best_exit_next_node = this->curr_exit_next_node;
			this->best_exit_throw_id = this->curr_exit_throw_id;
			this->best_catch_throw_ids = this->curr_catch_throw_ids;

			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_existing_scopes.clear();
			this->curr_potential_scopes.clear();
			this->curr_catch_throw_ids.clear();
		} else {
			for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
				if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->curr_actions[s_index];
				} else if (this->curr_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
					delete this->curr_existing_scopes[s_index];
				} else {
					delete this->curr_potential_scopes[s_index]->scope;
					delete this->curr_potential_scopes[s_index];
				}
			}

			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_existing_scopes.clear();
			this->curr_potential_scopes.clear();
			this->curr_catch_throw_ids.clear();
		}

		this->state_iter++;
		if (this->state_iter >= EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (this->best_surprise != 0.0) {
			#else
			if (this->best_surprise > 0.0) {
			#endif /* MDEBUG */
				// cout << "this->best_surprise: " << this->best_surprise << endl;

				// cout << "this->scope_context:" << endl;
				// for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				// 	cout << c_index << ": " << this->scope_context[c_index]->id << endl;
				// }
				// cout << "this->node_context:" << endl;
				// for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
				// 	cout << c_index << ": " << this->node_context[c_index]->id << endl;
				// }
				// cout << "this->is_branch: " << this->is_branch << endl;
				// cout << "this->throw_id: " << this->throw_id << endl;
				// cout << "new explore path:";
				// for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				// 	if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				// 		cout << " " << this->best_actions[s_index]->action.move;
				// 	} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
				// 		cout << " E" << this->best_existing_scopes[s_index]->scope->id;
				// 	} else {
				// 		cout << " P";
				// 	}
				// }
				// cout << endl;

				// cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
				// if (this->best_exit_next_node == NULL) {
				// 	cout << "this->best_exit_next_node->id: " << -1 << endl;
				// } else {
				// 	cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				// }
				// cout << "this->best_exit_throw_id: " << this->best_exit_throw_id << endl;
				// cout << endl;

				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->parent = this->scope_context.back();
						this->best_actions[s_index]->id = this->scope_context.back()->node_counter;
						this->scope_context.back()->node_counter++;
					} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
						this->best_existing_scopes[s_index]->parent = this->scope_context.back();
						this->best_existing_scopes[s_index]->id = this->scope_context.back()->node_counter;
						this->scope_context.back()->node_counter++;
					} else {
						this->best_potential_scopes[s_index]->parent = this->scope_context.back();
						this->best_potential_scopes[s_index]->id = this->scope_context.back()->node_counter;
						this->scope_context.back()->node_counter++;

						int new_scope_id = solution->scope_counter;
						solution->scope_counter++;
						this->best_potential_scopes[s_index]->scope->id = new_scope_id;
					}
				}

				int exit_node_id;
				AbstractNode* exit_node;
				if (this->best_exit_depth > 0
						|| this->best_exit_throw_id != -1) {
					ExitNode* new_exit_node = new ExitNode();
					new_exit_node->parent = this->scope_context.back();
					new_exit_node->id = this->scope_context.back()->node_counter;
					this->scope_context.back()->node_counter++;

					new_exit_node->exit_depth = this->best_exit_depth;
					new_exit_node->next_node_parent = this->scope_context[this->scope_context.size()-1 - this->best_exit_depth];
					if (this->best_exit_next_node == NULL) {
						new_exit_node->next_node_id = -1;
					} else {
						new_exit_node->next_node_id = this->best_exit_next_node->id;
					}
					new_exit_node->next_node = this->best_exit_next_node;
					if (this->best_exit_throw_id == TEMP_THROW_ID) {
						new_exit_node->throw_id = solution->throw_counter;
						solution->throw_counter++;
					} else {
						new_exit_node->throw_id = this->best_exit_throw_id;
					}

					this->exit_node = new_exit_node;

					exit_node_id = new_exit_node->id;
					exit_node = new_exit_node;
				} else {
					if (this->best_exit_next_node == NULL) {
						exit_node_id = -1;
					} else {
						exit_node_id = this->best_exit_next_node->id;
					}
					exit_node = this->best_exit_next_node;
				}

				/**
				 * - just need a placeholder for now
				 */
				this->branch_node = new BranchNode();
				this->branch_node->parent = this->scope_context.back();
				this->branch_node->id = this->scope_context.back()->node_counter;
				this->scope_context.back()->node_counter++;

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
						} else if (this->best_step_types[s_index+1] == STEP_TYPE_EXISTING_SCOPE) {
							next_node_id = this->best_existing_scopes[s_index+1]->id;
							next_node = this->best_existing_scopes[s_index+1];
						} else {
							next_node_id = this->best_potential_scopes[s_index+1]->id;
							next_node = this->best_potential_scopes[s_index+1];
						}
					}

					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->next_node_id = next_node_id;
						this->best_actions[s_index]->next_node = next_node;
					} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
						this->best_existing_scopes[s_index]->next_node_id = next_node_id;
						this->best_existing_scopes[s_index]->next_node = next_node;

						for (set<int>::iterator it = this->best_catch_throw_ids[s_index].begin();
								it != this->best_catch_throw_ids[s_index].end(); it++) {
							this->best_existing_scopes[s_index]->catch_ids[*it] = next_node_id;
							this->best_existing_scopes[s_index]->catches[*it] = next_node;
						}
					} else {
						this->best_potential_scopes[s_index]->next_node_id = next_node_id;
						this->best_potential_scopes[s_index]->next_node = next_node;

						for (set<int>::iterator it = this->best_catch_throw_ids[s_index].begin();
								it != this->best_catch_throw_ids[s_index].end(); it++) {
							this->best_potential_scopes[s_index]->catch_ids[*it] = next_node_id;
							this->best_potential_scopes[s_index]->catches[*it] = next_node;
						}
					}
				}

				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW;
				this->state_iter = 0;
				this->sub_state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
