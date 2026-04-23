#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "signal.h"
#include "signal_helpers.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"
#include "world_model.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 10;
#else
const int EXPLORE_ITERS = 200;
#endif /* MDEBUG */

void ExploreExperiment::explore_check_activate(SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->explore_experiment_history;

	this->num_instances_until_target--;
	if (history->existing_predicted_trues.size() == 0
			&& this->num_instances_until_target <= 0) {
		uniform_int_distribution<int> new_scope_distribution(0, 3);
		if (wrapper->solution->state == SOLUTION_STATE_OUTER) {
			this->curr_new_scope = outer_create_new_scope(wrapper);
		} else if (new_scope_distribution(generator) == 0) {
			this->curr_new_scope = create_new_scope(this->node_context->parent,
													wrapper);
		}
		if (this->curr_new_scope != NULL) {
			this->curr_step_types.push_back(STEP_TYPE_SCOPE);
			this->curr_actions.push_back(-1);
			this->curr_scopes.push_back(this->curr_new_scope);
		} else {
			bool exit_is_next;
			switch (this->node_context->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)this->node_context;
					if (this->exit_next_node == start_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					if (this->exit_next_node == action_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					if (this->exit_next_node == scope_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						if (this->exit_next_node == branch_node->branch_next_node) {
							exit_is_next = true;
						} else {
							exit_is_next = false;
						}
					} else {
						if (this->exit_next_node == branch_node->original_next_node) {
							exit_is_next = true;
						} else {
							exit_is_next = false;
						}
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)this->node_context;
					if (this->exit_next_node == obs_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			}

			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.3);
			/**
			 * - num_steps less than exit length on average to reduce solution size
			 */
			if (exit_is_next) {
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

		ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void ExploreExperiment::explore_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 bool& fetch_action,
									 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->explore_experiment_history;

		this->existing_true_network->activate(obs);
		history->existing_predicted_trues.push_back(
			this->existing_true_network->output->acti_vals[0]);
	}

	if (experiment_state->step_index >= (int)this->curr_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->curr_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void ExploreExperiment::explore_set_action(int action,
										   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	this->curr_actions[experiment_state->step_index] = action;

	experiment_state->step_index++;

	update_world_model(wrapper);
}

void ExploreExperiment::explore_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;

	update_world_model(wrapper);
}

void ExploreExperiment::explore_backprop(double target_val,
										 SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->explore_experiment_history;

	uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
	this->num_instances_until_target = until_distribution(generator);

	if (history->existing_predicted_trues.size() != 0) {
		update_signals_helper(target_val,
							  wrapper);

		// temp
		if (this->state_iter < 4) {
			wrapper->world_model->print();
			cout << "target_val: " << target_val << endl;
			double predicted = wrapper->solution->signal->activate(wrapper->world_model);
			cout << "predicted: " << predicted << endl;
			cout << endl;
		}

		double curr_surprise = target_val - history->existing_predicted_trues[0];

		#if defined(MDEBUG) && MDEBUG
		if (curr_surprise > this->best_surprise || true) {
		#else
		if (curr_surprise > this->best_surprise) {
		#endif /* MDEBUG */
			this->best_surprise = curr_surprise;
			if (this->best_new_scope != NULL) {
				delete this->best_new_scope;
			}
			this->best_new_scope = this->curr_new_scope;
			this->curr_new_scope = NULL;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_scopes = this->curr_scopes;
		}

		if (this->curr_new_scope != NULL) {
			delete this->curr_new_scope;
			this->curr_new_scope = NULL;
		}
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();

		this->state_iter++;
		if (this->state_iter >= EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->best_surprise >= 0.0) {
			#endif /* MDEBUG */
				this->sum_num_instances = 0;
				this->total_count = 0;

				this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_NEW;
				this->state_iter = 0;
			} else {
				wrapper->curr_explore_experiment = NULL;
				this->node_context->experiment = NULL;
				delete this;
			}
		}
	}
}
