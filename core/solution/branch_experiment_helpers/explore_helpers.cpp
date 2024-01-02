#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "solution_helpers.h"
#include "pass_through_experiment.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "try_instance.h"
#include "try_scope_step.h"
#include "try_tracker.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 5;
#else
const int EXPLORE_ITERS = 500;
#endif /* MDEBUG */

void BranchExperiment::explore_activate(AbstractNode*& curr_node,
										Problem* problem,
										vector<ContextLayer>& context,
										int& exit_depth,
										AbstractNode*& exit_node,
										RunHelper& run_helper) {
	bool is_target = false;
	BranchExperimentOverallHistory* overall_history;
	if (this->parent_pass_through_experiment != NULL) {
		PassThroughExperimentOverallHistory* parent_history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
		overall_history = parent_history->branch_experiment_history;
	} else {
		overall_history = (BranchExperimentOverallHistory*)run_helper.experiment_history;
	}
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

void BranchExperiment::explore_target_activate(AbstractNode*& curr_node,
											   Problem* problem,
											   vector<ContextLayer>& context,
											   int& exit_depth,
											   AbstractNode*& exit_node,
											   RunHelper& run_helper) {
	double predicted_score = this->existing_average_score;

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
			map<int, double>::iterator weight_it = this->existing_input_state_weights[c_index].find(it->first);
			if (weight_it != this->existing_input_state_weights[c_index].end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += weight_it->second * normalized;
				} else {
					predicted_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
			map<int, double>::iterator weight_it = this->existing_local_state_weights[c_index].find(it->first);
			if (weight_it != this->existing_local_state_weights[c_index].end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += weight_it->second * normalized;
				} else {
					predicted_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
			map<State*, double>::iterator weight_it = this->existing_temp_state_weights[c_index].find(it->first);
			if (weight_it != this->existing_temp_state_weights[c_index].end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += weight_it->second * normalized;
				} else {
					predicted_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	BranchExperimentOverallHistory* overall_history;
	if (this->parent_pass_through_experiment != NULL) {
		PassThroughExperimentOverallHistory* parent_history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
		overall_history = parent_history->branch_experiment_history;
	} else {
		overall_history = (BranchExperimentOverallHistory*)run_helper.experiment_history;
	}
	overall_history->existing_predicted_score = predicted_score;

	this->curr_try_instance = new TryInstance();

	uniform_int_distribution<int> loop_distribution(0, 1);
	if (this->parent_pass_through_experiment == NULL && loop_distribution(generator)) {
		this->curr_exit_depth = 0;
		this->curr_exit_node = curr_node;

		this->curr_step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
		this->curr_actions.push_back(NULL);

		this->curr_try_instance->step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
		this->curr_try_instance->actions.push_back(-1);

		PotentialScopeNode* new_potential_loop = new PotentialScopeNode();
		TryScopeStep* new_try_scope_step = new TryScopeStep();
		create_loop(context,
					(int)this->scope_context.size(),
					new_potential_loop,
					new_try_scope_step);
		this->curr_potential_scopes.push_back(new_potential_loop);
		this->curr_try_instance->potential_scopes.push_back(new_try_scope_step);

		PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(new_potential_loop);
		new_potential_loop->activate(problem,
									 context,
									 run_helper,
									 potential_scope_node_history);
		delete potential_scope_node_history;

		this->curr_is_loop = true;

		curr_node = this->curr_exit_node;
	} else {
		{
			// exit
			vector<pair<int,AbstractNode*>> possible_exits;
			if (this->parent_pass_through_experiment == NULL) {
				gather_possible_exits(possible_exits,
									  context,
									  this->scope_context,
									  this->node_context);
			} else {
				parent_pass_through_gather_possible_exits(
					possible_exits,
					context,
					this->parent_pass_through_experiment->scope_context,
					this->parent_pass_through_experiment->node_context,
					this->parent_pass_through_experiment->best_exit_depth,
					this->parent_pass_through_experiment->best_exit_node);

				for (int s_index = this->parent_pass_through_experiment->branch_experiment_step_index+1;
						s_index < (int)this->parent_pass_through_experiment->best_step_types.size(); s_index++) {
					if (this->parent_pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
						possible_exits.push_back({0, this->parent_pass_through_experiment->best_actions[s_index]});
					} else {
						possible_exits.push_back({0, this->parent_pass_through_experiment->best_potential_scopes[s_index]->scope_node_placeholder});
					}
				}
			}

			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			int random_index = distribution(generator);
			this->curr_exit_depth = possible_exits[random_index].first;
			this->curr_exit_node = possible_exits[random_index].second;
		}

		{
			// new path
			int new_num_steps;
			uniform_int_distribution<int> uniform_distribution(0, 2);
			geometric_distribution<int> geometric_distribution(0.5);
			if (this->curr_exit_depth == 0
					&& this->curr_exit_node == curr_node) {
				new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
			} else {
				new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
			}
		
			uniform_int_distribution<int> type_distribution(0, 1);
			uniform_int_distribution<int> next_distribution(0, 1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				if (type_distribution(generator) == 0) {
					this->curr_step_types.push_back(STEP_TYPE_ACTION);

					ActionNode* new_action_node = new ActionNode();
					new_action_node->action = problem->random_action();
					this->curr_actions.push_back(new_action_node);

					this->curr_potential_scopes.push_back(NULL);

					this->curr_try_instance->step_types.push_back(STEP_TYPE_ACTION);
					this->curr_try_instance->actions.push_back(new_action_node->action.move);
					this->curr_try_instance->potential_scopes.push_back(NULL);

					ActionNodeHistory* action_node_history = new ActionNodeHistory(new_action_node);
					new_action_node->activate(curr_node,
											  problem,
											  context,
											  exit_depth,
											  exit_node,
											  run_helper,
											  action_node_history);
					delete action_node_history;
				} else {
					this->curr_step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
					this->curr_actions.push_back(NULL);

					this->curr_try_instance->step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
					this->curr_try_instance->actions.push_back(-1);

					PotentialScopeNode* new_potential_scope_node = new PotentialScopeNode();
					TryScopeStep* new_try_scope_step = new TryScopeStep();
					create_scope(context,
								 (int)this->scope_context.size(),
								 context[context.size() - this->scope_context.size()].scope,
								 this->parent_pass_through_experiment,
								 new_potential_scope_node,
								 new_try_scope_step);
					this->curr_potential_scopes.push_back(new_potential_scope_node);
					this->curr_try_instance->potential_scopes.push_back(new_try_scope_step);

					PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(new_potential_scope_node);
					new_potential_scope_node->activate(problem,
													   context,
													   run_helper,
													   potential_scope_node_history);
					delete potential_scope_node_history;
				}
			}
		}
		this->curr_is_loop = false;

		if (this->curr_exit_node == NULL) {
			this->curr_try_instance->exit = {this->curr_exit_depth, {-1, -1}};
		} else {
			this->curr_try_instance->exit = {this->curr_exit_depth, {this->curr_exit_node->parent->id, this->curr_exit_node->id}};
		}

		{
			if (this->curr_exit_depth == 0) {
				curr_node = this->curr_exit_node;
			} else {
				curr_node = NULL;

				exit_depth = this->curr_exit_depth-1;
				exit_node = this->curr_exit_node;
			}
		}
	}
}

void BranchExperiment::explore_backprop(double target_val,
										BranchExperimentOverallHistory* history) {
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
				} else {
					delete this->best_potential_scopes[s_index];
				}
			}
			if (this->best_try_instance != NULL) {
				delete this->best_try_instance;
			}

			this->best_surprise = curr_surprise;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_potential_scopes = this->curr_potential_scopes;
			this->best_exit_depth = this->curr_exit_depth;
			this->best_exit_node = this->curr_exit_node;
			this->best_is_loop = this->curr_is_loop;
			this->best_try_instance = this->curr_try_instance;

			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_potential_scopes.clear();
			this->curr_try_instance = NULL;
		} else {
			for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
				if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->curr_actions[s_index];
				} else {
					delete this->curr_potential_scopes[s_index];
				}
			}
			delete this->curr_try_instance;

			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_potential_scopes.clear();
			this->curr_try_instance = NULL;
		}

		this->state_iter++;
		if (this->state_iter >= EXPLORE_ITERS) {
			// cout << "Branch" << endl;
			/**
			 * - if surprise isn't better than 0.0, don't bother
			 */
			// cout << "this->best_surprise: " << this->best_surprise << endl;
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->best_surprise > 0.0) {
			#endif /* MDEBUG */
				if (this->parent_pass_through_experiment == NULL) {
					Scope* containing_scope = solution->scopes[this->scope_context.back()];
					if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)containing_scope->nodes[this->node_context.back()];

						map<pair<vector<int>,vector<int>>, TryTracker*>::iterator it = action_node->tries.find(
							{this->scope_context, this->node_context});
						if (it != action_node->tries.end()) {
							double predicted_impact;
							double closest_distance;
							it->second->evaluate_potential(this->best_try_instance,
														   predicted_impact,
														   closest_distance);
							cout << "predicted_impact: " << predicted_impact << endl;
							cout << "closest_distance: " << closest_distance << endl;
						}
					} else {
						ScopeNode* scope_node = (ScopeNode*)containing_scope->nodes[this->node_context.back()];

						map<pair<vector<int>,vector<int>>, TryTracker*>::iterator it = scope_node->tries.find(
							{this->scope_context, this->node_context});
						if (it != scope_node->tries.end()) {
							double predicted_impact;
							double closest_distance;
							it->second->evaluate_potential(this->best_try_instance,
														   predicted_impact,
														   closest_distance);
							cout << "predicted_impact: " << predicted_impact << endl;
							cout << "closest_distance: " << closest_distance << endl;
						}
					}
				}

				Scope* containing_scope = solution->scopes[this->scope_context.back()];
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->parent = containing_scope;
						this->best_actions[s_index]->id = containing_scope->node_counter;
						containing_scope->node_counter++;
					} else {
						this->best_potential_scopes[s_index]->scope_node_placeholder = new ScopeNode();
						this->best_potential_scopes[s_index]->scope_node_placeholder->parent = containing_scope;
						this->best_potential_scopes[s_index]->scope_node_placeholder->id = containing_scope->node_counter;
						containing_scope->node_counter++;

						int new_scope_id = solution->scope_counter;
						solution->scope_counter++;
						this->best_potential_scopes[s_index]->scope->id = new_scope_id;

						for (map<int, AbstractNode*>::iterator it = this->best_potential_scopes[s_index]->scope->nodes.begin();
								it != this->best_potential_scopes[s_index]->scope->nodes.end(); it++) {
							if (it->second->type == NODE_TYPE_BRANCH) {
								BranchNode* branch_node = (BranchNode*)it->second;
								branch_node->branch_scope_context = vector<int>{new_scope_id};
								branch_node->branch_node_context = vector<int>{branch_node->id};
							}
						}
					}
				}

				// cout << "this->scope_context:" << endl;
				// for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				// 	cout << c_index << ": " << this->scope_context[c_index] << endl;
				// }
				// cout << "this->node_context:" << endl;
				// for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
				// 	cout << c_index << ": " << this->node_context[c_index] << endl;
				// }
				// cout << "new explore path:";
				// for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				// 	if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				// 		cout << " " << this->best_actions[s_index]->action.move;
				// 	} else {
				// 		cout << " S";
				// 	}
				// }
				// cout << endl;

				// cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
				// if (this->best_exit_node == NULL) {
				// 	cout << "this->best_exit_node_id: " << -1 << endl;
				// } else {
				// 	cout << "this->best_exit_node_id: " << this->best_exit_node->id << endl;
				// }
				// cout << endl;

				// cout << "this->best_is_loop: " << this->best_is_loop << endl;

				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW_PRE;
				this->state_iter = 0;
			} else {
				this->state = BRANCH_EXPERIMENT_STATE_FAIL;
			}
		}
	}
}
