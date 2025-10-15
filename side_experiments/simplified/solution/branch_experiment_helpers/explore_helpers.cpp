/**
 * - discard explore sample
 *   - would add bias to training
 */

#include "branch_experiment.h"

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
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 10;
#else
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 200;
#endif /* MDEBUG */

/**
 * - simply give raw actions a fixed weight
 *   - cannot track success/count if continuous
 *   - raw actions can also drive innovation anyways
 */
const int RAW_ACTION_WEIGHT = 10;

void BranchExperiment::explore_check_activate(
		SolutionWrapper* wrapper,
		BranchExperimentHistory* history) {
	this->num_instances_until_target--;
	if (history->existing_predicted_scores.size() == 0
			&& this->num_instances_until_target <= 0) {
		double sum_vals = this->existing_constant;
		for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(wrapper->scope_histories.back(),
							   this->existing_inputs[i_index],
							   0,
							   val,
							   is_on);
			if (is_on) {
				double normalized_val = (val - this->existing_input_averages[i_index]) / this->existing_input_standard_deviations[i_index];
				sum_vals += this->existing_weights[i_index] * normalized_val;
			}
		}
		if (this->existing_network != NULL) {
			vector<double> input_vals(this->existing_network_inputs.size());
			vector<bool> input_is_on(this->existing_network_inputs.size());
			for (int i_index = 0; i_index < (int)this->existing_network_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(wrapper->scope_histories.back(),
								   this->existing_network_inputs[i_index],
								   0,
								   val,
								   is_on);
				input_vals[i_index] = val;
				input_is_on[i_index] = is_on;
			}
			this->existing_network->activate(input_vals,
										input_is_on);
			sum_vals += this->existing_network->output->acti_vals[0];
		}
		history->existing_predicted_scores.push_back(sum_vals);

		vector<AbstractNode*> possible_exits;

		AbstractNode* starting_node;
		switch (this->node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->node_context;
				starting_node = start_node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				starting_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;
				starting_node = obs_node->next_node;
			}
			break;
		}

		this->scope_context->random_exit_activate(
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

		#if defined(MDEBUG) && MDEBUG
		uniform_int_distribution<int> new_scope_distribution(0, 1);
		#else
		uniform_int_distribution<int> new_scope_distribution(0, 4);
		#endif /* MDEBUG */
		if (new_scope_distribution(generator) == 0) {
			this->curr_new_scope = create_new_scope(this->node_context->parent);
		}
		if (this->curr_new_scope != NULL) {
			this->curr_step_types.push_back(STEP_TYPE_SCOPE);
			this->curr_actions.push_back(-1);
			this->curr_scopes.push_back(this->curr_new_scope);
		} else {
			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.3);
			/**
			 * - num_steps less than exit length on average to reduce solution size
			 */
			if (random_index == 0) {
				new_num_steps = 1 + geo_distribution(generator);
			} else {
				new_num_steps = geo_distribution(generator);
			}

			vector<int> possible_child_indexes;
			vector<double> possible_child_index_weights;
			double sum_weights = 0.0;
			for (int c_index = 0; c_index < (int)this->node_context->parent->child_scopes.size(); c_index++) {
				if (this->node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
					possible_child_indexes.push_back(c_index);

					double weight = sqrt(this->node_context->parent->child_scope_successes[c_index])
						/ log(this->node_context->parent->child_scope_tries[c_index]);
					possible_child_index_weights.push_back(weight);
					sum_weights += weight;
				}
			}
			uniform_real_distribution<double> child_index_distribution(0.0, sum_weights);
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

					double rand_val = child_index_distribution(generator);
					for (int c_index = 0; c_index < (int)possible_child_indexes.size(); c_index++) {
						rand_val -= possible_child_index_weights[c_index];
						if (rand_val <= 0.0) {
							int child_index = possible_child_indexes[c_index];
							this->curr_scopes.push_back(this->node_context->parent->child_scopes[child_index]);
							this->node_context->parent->child_scope_tries[child_index]++;
							break;
						}
					}
				} else {
					this->curr_step_types.push_back(STEP_TYPE_ACTION);

					this->curr_actions.push_back(-1);

					this->curr_scopes.push_back(NULL);
				}
			}
		}

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void BranchExperiment::explore_step(vector<double>& obs,
									int& action,
									bool& is_next,
									bool& fetch_action,
									SolutionWrapper* wrapper,
									BranchExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->curr_step_types.size()) {
		wrapper->node_context.back() = this->curr_exit_next_node;

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

void BranchExperiment::explore_set_action(int action,
										  BranchExperimentState* experiment_state) {
	this->curr_actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void BranchExperiment::explore_exit_step(SolutionWrapper* wrapper,
										 BranchExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::explore_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	if (history->existing_predicted_scores.size() > 0) {
		double curr_surprise = (target_val - wrapper->solution->curr_score)
			- history->existing_predicted_scores[0];

		#if defined(MDEBUG) && MDEBUG
		if (true) {
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
		if (this->state_iter >= BRANCH_EXPERIMENT_EXPLORE_ITERS) {
			if (this->best_surprise >= 0.0) {
				uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
				this->num_instances_until_target = until_distribution(generator);

				this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
