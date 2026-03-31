/**
 * - discard explore sample
 *   - would add bias to training
 */

#include "experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_EXPLORE_ITERS = 5;
#else
const int EXPERIMENT_EXPLORE_ITERS = 200;
#endif /* MDEBUG */

void Experiment::explore_check_activate(
		vector<double>& obs,
		bool& is_next,
		bool& is_done,
		SolutionWrapper* wrapper,
		ExperimentHistory* history) {
	/**
	 * - still only activate some of the time to prevent correlation with other experiments
	 */
	uniform_int_distribution<int> on_distribution(0, 9);
	if (wrapper->is_explore
			&& wrapper->num_actions >= wrapper->is_explore_num_actions
			&& on_distribution(generator) == 0) {
		AbstractNode* starting_node = this->node_context->next_node;
		vector<AbstractNode*> possible_exits;
		this->node_context->parent->random_exit_activate(
			starting_node,
			possible_exits);

		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		this->curr_exit_next_node = possible_exits[random_index];

		uniform_int_distribution<int> new_scope_distribution(0, 3);
		if (this->node_context->parent->id != -1
				&& !this->node_context->parent->is_outer
				&& wrapper->solution->state == SOLUTION_STATE_OUTER) {
			Scope* parent_scope;
			outer_create_new_scope(this->node_context->parent,
								   wrapper,
								   this->curr_new_scope,
								   parent_scope);
		} else if (this->node_context->parent->id != -1
				&& new_scope_distribution(generator) == 0) {
			Scope* parent_scope;
			create_new_scope(this->node_context->parent,
							 wrapper,
							 this->curr_new_scope,
							 parent_scope);
		}
		if (this->curr_new_scope != NULL) {
			this->curr_step_types.push_back(STEP_TYPE_SCOPE);
			this->curr_actions.push_back(-1);
			this->curr_scopes.push_back(this->curr_new_scope);
		} else {
			bool is_in_place;
			if (this->curr_exit_next_node == this->node_context->next_node) {
				is_in_place = true;
			} else {
				is_in_place = false;
			}

			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.3);
			/**
			 * - num_steps less than exit length on average to reduce solution size
			 */
			if (is_in_place) {
				new_num_steps = 1 + geo_distribution(generator);
			} else {
				new_num_steps = geo_distribution(generator);
			}

			vector<int> possible_child_indexes;
			for (int c_index = 0; c_index < (int)this->node_context->parent->child_scopes.size(); c_index++) {
				if (this->node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
					possible_child_indexes.push_back(c_index);
				}
			}
			uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				bool is_scope = false;
				if (possible_child_indexes.size() > 0) {
					if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
						uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
						if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
							is_scope = true;
						}
					} else {
						uniform_int_distribution<int> scope_distribution(0, 1);
						if (scope_distribution(generator) == 0) {
							is_scope = true;
						}
					}
				}
				if (is_scope) {
					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(-1);

					int child_index = possible_child_indexes[child_index_distribution(generator)];
					this->curr_scopes.push_back(this->node_context->parent->child_scopes[child_index]);
				} else {
					this->curr_step_types.push_back(STEP_TYPE_ACTION);

					this->curr_actions.push_back(-1);

					this->curr_scopes.push_back(NULL);
				}
			}
		}

		ExperimentState* new_experiment_state = new ExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;

		double next_clean_result = clean_result_helper(wrapper);
		this->curr_surprise = next_clean_result - wrapper->prev_clean_result;
		wrapper->prev_clean_result = next_clean_result;
	}
}

void Experiment::explore_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  bool& fetch_action,
							  SolutionWrapper* wrapper,
							  ExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->curr_step_types.size()) {
		wrapper->node_context.back() = this->curr_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;

		#if defined(MDEBUG) && MDEBUG
		if (true) {
		#else
		if (this->curr_surprise > this->best_surprise) {
		#endif /* MDEBUG */
			this->best_surprise = this->curr_surprise;
			if (this->best_new_scope != NULL) {
				delete this->best_new_scope;
			}
			this->best_new_scope = this->curr_new_scope;
			this->curr_new_scope = NULL;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_scopes = this->curr_scopes;
			this->best_exit_next_node = this->curr_exit_next_node;
		}

		if (this->curr_new_scope != NULL) {
			delete this->curr_new_scope;
			this->curr_new_scope = NULL;
		}
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();

		this->state_iter++;
	} else {
		if (this->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			if (this->curr_actions[experiment_state->step_index] == -1) {
				is_next = true;
				fetch_action = true;

				wrapper->num_actions++;
			} else {
				action = this->curr_actions[experiment_state->step_index];
				is_next = true;

				wrapper->num_actions++;

				experiment_state->step_index++;
			}
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->curr_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Experiment::explore_set_action(int action,
									ExperimentState* experiment_state) {
	this->curr_actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void Experiment::explore_exit_step(SolutionWrapper* wrapper,
								   ExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::explore_backprop(double target_val,
								  ExperimentHistory* history,
								  SolutionWrapper* wrapper) {
	/**
	 * - in case inner early exit
	 */
	if (this->curr_new_scope != NULL) {
		delete this->curr_new_scope;
		this->curr_new_scope = NULL;
	}
	this->curr_step_types.clear();
	this->curr_actions.clear();
	this->curr_scopes.clear();

	if (this->state_iter >= EXPERIMENT_EXPLORE_ITERS) {
		#if defined(MDEBUG) && MDEBUG
		if (this->best_surprise > 0.0 || true) {
		#else
		if (this->best_surprise > 0.0) {
		#endif /* MDEBUG */
			this->starting_iter = wrapper->iter;

			this->state = EXPERIMENT_STATE_TRAIN_NEW;
			this->state_iter = 0;
		} else {
			this->node_context->experiment = NULL;
			delete this;
		}
	}
}

void Experiment::result_explore_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 bool& fetch_action,
									 SolutionWrapper* wrapper,
									 ExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->curr_step_types.size()) {
		wrapper->result_node_context.back() = this->curr_exit_next_node;

		delete experiment_state;
		wrapper->result_experiment_context.back() = NULL;
	} else {
		if (this->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			if (this->curr_actions[experiment_state->step_index] == -1) {
				is_next = true;
				fetch_action = true;

				wrapper->result_num_actions++;
			} else {
				action = this->curr_actions[experiment_state->step_index];
				is_next = true;

				wrapper->result_num_actions++;

				experiment_state->step_index++;
			}
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[experiment_state->step_index]);
			wrapper->result_scope_histories.push_back(inner_scope_history);
			wrapper->result_node_context.push_back(this->curr_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->result_experiment_context.push_back(NULL);
		}
	}
}

void Experiment::result_explore_set_action(int action,
										   ExperimentState* experiment_state) {
	this->curr_actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void Experiment::result_explore_exit_step(SolutionWrapper* wrapper,
										  ExperimentState* experiment_state) {
	delete wrapper->result_scope_histories.back();

	wrapper->result_scope_histories.pop_back();
	wrapper->result_node_context.pop_back();
	wrapper->result_experiment_context.pop_back();

	experiment_state->step_index++;
}
