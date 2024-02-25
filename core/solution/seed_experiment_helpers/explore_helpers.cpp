#include "seed_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "problem.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "seed_experiment_filter.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 5;
#else
const int EXPLORE_ITERS = 500;
#endif /* MDEBUG */

void SeedExperiment::explore_activate(AbstractNode*& curr_node,
									  Problem* problem,
									  vector<ContextLayer>& context,
									  int& exit_depth,
									  AbstractNode*& exit_node,
									  RunHelper& run_helper) {
	bool is_target = false;
	SeedExperimentOverallHistory* overall_history = (SeedExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->instance_count++;
	if (!overall_history->has_target) {
		double target_probability;
		if (overall_history->instance_count > this->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - overall_history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		overall_history->has_target = true;

		explore_target_activate(curr_node,
								problem,
								context,
								exit_depth,
								exit_node,
								run_helper);
	}
}

void SeedExperiment::explore_target_activate(AbstractNode*& curr_node,
											 Problem* problem,
											 vector<ContextLayer>& context,
											 int& exit_depth,
											 AbstractNode*& exit_node,
											 RunHelper& run_helper) {
	vector<double> input_vals(this->existing_input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->existing_input_scope_contexts.size(); i_index++) {
		if (this->existing_input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->existing_input_node_contexts[i_index].back();
			action_node->hook_indexes.push_back(i_index);
			action_node->hook_scope_contexts.push_back(this->existing_input_scope_contexts[i_index]);
			action_node->hook_node_contexts.push_back(this->existing_input_node_contexts[i_index]);
		} else {
			BranchNode* branch_node = (BranchNode*)this->existing_input_node_contexts[i_index].back();
			branch_node->hook_indexes.push_back(i_index);
			branch_node->hook_scope_contexts.push_back(this->existing_input_scope_contexts[i_index]);
			branch_node->hook_node_contexts.push_back(this->existing_input_node_contexts[i_index]);
		}
	}
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	input_vals_helper(scope_context,
					  node_context,
					  input_vals,
					  context[context.size() - this->scope_context.size()].scope_history);
	for (int i_index = 0; i_index < (int)this->existing_input_scope_contexts.size(); i_index++) {
		if (this->existing_input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->existing_input_node_contexts[i_index].back();
			action_node->hook_indexes.clear();
			action_node->hook_scope_contexts.clear();
			action_node->hook_node_contexts.clear();
		} else {
			BranchNode* branch_node = (BranchNode*)this->existing_input_node_contexts[i_index].back();
			branch_node->hook_indexes.clear();
			branch_node->hook_scope_contexts.clear();
			branch_node->hook_node_contexts.clear();
		}
	}

	double predicted_score = this->existing_average_score;
	for (int l_index = 0; l_index < (int)this->existing_linear_weights.size(); l_index++) {
		predicted_score += input_vals[l_index] * this->existing_linear_weights[l_index];
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

	SeedExperimentOverallHistory* overall_history = (SeedExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->existing_predicted_score = predicted_score;

	// exit
	vector<pair<int,AbstractNode*>> possible_exits;
	gather_possible_exits(possible_exits,
						  this->scope_context,
						  this->node_context,
						  this->is_branch);

	uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
	int random_index = distribution(generator);
	this->curr_exit_depth = possible_exits[random_index].first;
	this->curr_exit_next_node = possible_exits[random_index].second;

	// new path
	int new_num_steps;
	uniform_int_distribution<int> uniform_distribution(0, 2);
	geometric_distribution<int> geometric_distribution(0.5);
	if (this->curr_exit_depth == 0
			&& this->curr_exit_next_node == curr_node) {
		new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
	} else {
		new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
	}

	uniform_int_distribution<int> new_scope_distribution(0, 3);
	uniform_int_distribution<int> random_scope_distribution(0, 3);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		ScopeNode* new_scope_node = NULL;
		if (new_scope_distribution(generator) == 0) {
			if (random_scope_distribution(generator) == 0) {
				uniform_int_distribution<int> distribution(0, solution->scopes.size()-1);
				Scope* scope = next(solution->scopes.begin(), distribution(generator))->second;
				new_scope_node = create_scope(scope,
											  run_helper);
			} else {
				new_scope_node = create_scope(context[context.size() - this->scope_context.size()].scope,
											  run_helper);
			}
		}
		if (new_scope_node != NULL) {
			this->curr_step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
			this->curr_actions.push_back(NULL);
			this->curr_existing_scopes.push_back(NULL);

			this->curr_potential_scopes.push_back(new_scope_node);
		} else {
			ScopeNode* new_existing_scope_node = reuse_existing();
			if (new_existing_scope_node != NULL) {
				this->curr_step_types.push_back(STEP_TYPE_EXISTING_SCOPE);
				this->curr_actions.push_back(NULL);

				this->curr_existing_scopes.push_back(new_existing_scope_node);

				this->curr_potential_scopes.push_back(NULL);
			} else {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem->random_action();
				this->curr_actions.push_back(new_action_node);

				this->curr_existing_scopes.push_back(NULL);
				this->curr_potential_scopes.push_back(NULL);
			}
		}
	}

	// {
	// 	this->curr_step_types.push_back(STEP_TYPE_ACTION);

	// 	ActionNode* new_action_node = new ActionNode();
	// 	// LEFT
	// 	new_action_node->action = Action(3);
	// 	this->curr_actions.push_back(new_action_node);

	// 	this->curr_existing_scopes.push_back(NULL);
	// 	this->curr_potential_scopes.push_back(NULL);
	// }
	// {
	// 	this->curr_step_types.push_back(STEP_TYPE_ACTION);

	// 	ActionNode* new_action_node = new ActionNode();
	// 	// LEFT
	// 	new_action_node->action = Action(3);
	// 	this->curr_actions.push_back(new_action_node);

	// 	this->curr_existing_scopes.push_back(NULL);
	// 	this->curr_potential_scopes.push_back(NULL);
	// }
	// {
	// 	this->curr_step_types.push_back(STEP_TYPE_ACTION);

	// 	ActionNode* new_action_node = new ActionNode();
	// 	// UP
	// 	new_action_node->action = Action(0);
	// 	this->curr_actions.push_back(new_action_node);

	// 	this->curr_existing_scopes.push_back(NULL);
	// 	this->curr_potential_scopes.push_back(NULL);
	// }
	// {
	// 	this->curr_step_types.push_back(STEP_TYPE_ACTION);

	// 	ActionNode* new_action_node = new ActionNode();
	// 	// FLAG
	// 	new_action_node->action = Action(5);
	// 	this->curr_actions.push_back(new_action_node);

	// 	this->curr_existing_scopes.push_back(NULL);
	// 	this->curr_potential_scopes.push_back(NULL);
	// }

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
	}

	if (this->curr_exit_depth == 0) {
		curr_node = this->curr_exit_next_node;
	} else {
		exit_depth = this->curr_exit_depth-1;
		exit_node = this->curr_exit_next_node;
	}
}

void SeedExperiment::explore_backprop(double target_val,
									  SeedExperimentOverallHistory* history) {
	if (history->has_target) {
		double curr_surprise = target_val - history->existing_predicted_score;
		#if defined(MDEBUG) && MDEBUG
		if (true) {
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

			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_existing_scopes.clear();
			this->curr_potential_scopes.clear();
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
		}

		this->state_iter++;
		if (this->state_iter >= EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->best_surprise > this->existing_score_standard_deviation) {
			#endif /* MDEBUG */
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

						for (map<int, AbstractNode*>::iterator it = this->best_potential_scopes[s_index]->scope->nodes.begin();
								it != this->best_potential_scopes[s_index]->scope->nodes.end(); it++) {
							if (it->second->type == NODE_TYPE_BRANCH) {
								BranchNode* branch_node = (BranchNode*)it->second;
								branch_node->scope_context_ids[0] = new_scope_id;
								for (int i_index = 0; i_index < (int)branch_node->input_scope_context_ids.size(); i_index++) {
									if (branch_node->input_scope_context_ids[i_index].size() > 0) {
										branch_node->input_scope_context_ids[i_index][0] = new_scope_id;
									}
								}
							}
						}
					}
				}

				int end_node_id;
				AbstractNode* end_node;
				if (this->best_exit_depth > 0) {
					ExitNode* new_exit_node = new ExitNode();
					new_exit_node->parent = this->scope_context.back();
					new_exit_node->id = this->scope_context.back()->node_counter;
					this->scope_context.back()->node_counter++;

					new_exit_node->exit_depth = this->best_exit_depth;
					new_exit_node->next_node_parent_id = this->scope_context[this->scope_context.size()-1 - this->best_exit_depth]->id;
					if (this->best_exit_next_node == NULL) {
						new_exit_node->next_node_id = -1;
					} else {
						new_exit_node->next_node_id = this->best_exit_next_node->id;
					}
					new_exit_node->next_node = this->best_exit_next_node;

					this->best_exit_node = new_exit_node;

					end_node_id = new_exit_node->id;
					end_node = new_exit_node;
				} else {
					if (this->best_exit_next_node == NULL) {
						end_node_id = -1;
					} else {
						end_node_id = this->best_exit_next_node->id;
					}
					end_node = this->best_exit_next_node;
				}

				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					int next_node_id;
					AbstractNode* next_node;
					if (s_index == (int)this->best_step_types.size()-1) {
						next_node_id = end_node_id;
						next_node = end_node;
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
					} else {
						this->best_potential_scopes[s_index]->next_node_id = next_node_id;
						this->best_potential_scopes[s_index]->next_node = next_node;
					}
				}

				this->curr_higher_ratio = 0.0;

				AbstractNode* seed_next_node;
				if (this->best_step_types.size() == 0) {
					seed_next_node = best_exit_next_node;
				} else {
					if (this->best_step_types[0] == STEP_TYPE_ACTION) {
						seed_next_node = this->best_actions[0];
					} else if (this->best_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
						seed_next_node = this->best_existing_scopes[0];
					} else {
						seed_next_node = this->best_potential_scopes[0];
					}
				}
				AbstractNode* filter_next_node;
				if (this->node_context.back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->node_context.back();
					filter_next_node = action_node->next_node;
				} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
					filter_next_node = scope_node->next_node;
				} else {
					BranchNode* branch_node = (BranchNode*)this->node_context.back();
					if (this->is_branch) {
						filter_next_node = branch_node->branch_next_node;
					} else {
						filter_next_node = branch_node->original_next_node;
					}
				}
				this->curr_filter = new SeedExperimentFilter(this,
															 this->scope_context,
															 this->node_context,
															 this->is_branch,
															 seed_next_node,
															 vector<int>(),
															 vector<ActionNode*>(),
															 vector<ScopeNode*>(),
															 vector<ScopeNode*>(),
															 0,
															 filter_next_node);
				if (this->node_context.back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->node_context.back();
					action_node->experiments.insert(action_node->experiments.begin(), this->curr_filter);
				} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
					scope_node->experiments.insert(scope_node->experiments.begin(), this->curr_filter);
				} else {
					BranchNode* branch_node = (BranchNode*)this->node_context.back();
					branch_node->experiments.insert(branch_node->experiments.begin(), this->curr_filter);
					branch_node->experiment_types.insert(branch_node->experiment_types.begin(), this->is_branch);
				}

				this->curr_filter_score = 0.0;
				this->curr_filter_step_index = 0;
				this->curr_filter_is_success = false;

				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_is_higher_histories.reserve(solution->curr_num_datapoints);

				cout << "SEED_EXPERIMENT_STATE_TRAIN_FILTER" << endl;
				this->state = SEED_EXPERIMENT_STATE_TRAIN_FILTER;
				this->state_iter = 0;
				this->sub_state_iter = 0;
			} else {
				cout << "EXPERIMENT_RESULT_FAIL" << endl;
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
