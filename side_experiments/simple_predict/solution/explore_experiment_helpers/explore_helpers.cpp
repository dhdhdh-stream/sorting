#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "predict_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MIN_NUM_SAMPLES = 2;
const double BETTER_THAN_RATIO = 0.5;
#else
const int MIN_NUM_SAMPLES = 5;
const double BETTER_THAN_RATIO = 0.9;
#endif /* MDEBUG */

void ExploreExperiment::explore_check_activate(vector<double>& obs,
											   ExploreExperimentHistory* history,
											   SolutionWrapper* wrapper) {
	if (wrapper->diversity_index == this->diversity_index) {
		this->num_instances_until_target--;
		if (!history->has_explore
				&& this->num_instances_until_target <= 0) {
			history->has_explore = true;

			wrapper->has_explore = true;
			ScopeHistory* scope_history = wrapper->scope_histories.back();
			scope_history->explore_index = (int)scope_history->node_histories.size()-1;

			this->existing_network->activate(obs);
			history->existing_predicted = this->existing_network->output->acti_vals[0];

			bool exit_is_next;
			switch (this->node_context->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)this->node_context;
					if (this->exit_next_node == noop_node->next_node) {
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
			default:
			// case NODE_TYPE_BRANCH:
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
			uniform_int_distribution<int> action_distribution(0, this->scope_context->generic_action_nodes.size()-1);
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
					history->curr_step_types.push_back(STEP_TYPE_SCOPE);
					int child_index = possible_child_indexes[child_index_distribution(generator)];
					history->curr_indexes.push_back(child_index);
				} else {
					history->curr_step_types.push_back(STEP_TYPE_ACTION);
					history->curr_indexes.push_back(action_distribution(generator));
				}
			}

			Eigen::VectorXf state;
			state.resize(NUM_STATES);
			state.setConstant(0.0);
			calc_curr_state_helper(wrapper->scope_histories[0],
								   state);
			for (int s_index = 0; s_index < (int)history->curr_step_types.size(); s_index++) {
				if (history->curr_step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNode* generic_action_node = this->scope_context->generic_action_nodes[history->curr_indexes[s_index]];
					generic_action_node->predict_network->activate(state);
				} else {
					ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[history->curr_indexes[s_index]];
					generic_scope_node->predict_network->activate(state);
				}
			}
			AbstractNode* curr_node = this->exit_next_node;
			while (curr_node != NULL) {
				curr_node->predict_step(state,
										curr_node);
			}
			this->scope_context->score_network->activate(state);
			history->predicted = this->scope_context->score_network->output->acti_vals(0);

			ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void ExploreExperiment::explore_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_histories[this->diversity_index][this];

	if (experiment_state->step_index >= (int)history->curr_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (history->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = history->curr_indexes[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;
		} else {
			ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[
				history->curr_indexes[experiment_state->step_index]];
			generic_scope_node->experiment_step(obs,
												action,
												is_next,
												wrapper);
		}
	}
}

void ExploreExperiment::explore_callback(vector<double>& obs,
										 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_histories[this->diversity_index][this];

	int action = history->curr_indexes[experiment_state->step_index];
	ActionNode* generic_action_node = this->scope_context->generic_action_nodes[action];
	generic_action_node->experiment_step_callback(obs,
												  wrapper);

	experiment_state->step_index++;
}

void ExploreExperiment::explore_exit_step(vector<double>& obs,
										  SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_histories[this->diversity_index][this];

	ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[
		history->curr_indexes[experiment_state->step_index]];
	generic_scope_node->experiment_exit_step(obs,
											 wrapper);

	experiment_state->step_index++;
}

void ExploreExperiment::explore_backprop(double target_val,
										 ExploreExperimentHistory* history,
										 SolutionWrapper* wrapper) {
	if (wrapper->diversity_index == this->diversity_index) {
		double average_instances_per_hit;
		switch (this->node_context->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)this->node_context;
				average_instances_per_hit = noop_node->average_instances_per_hit;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				average_instances_per_hit = action_node->average_instances_per_hit;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				average_instances_per_hit = scope_node->average_instances_per_hit;
			}
			break;
		default:
		// case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					average_instances_per_hit = branch_node->branch_average_instances_per_hit;
				} else {
					average_instances_per_hit = branch_node->original_average_instances_per_hit;
				}
			}
			break;
		}
		uniform_int_distribution<int> until_distribution(1, 2 * average_instances_per_hit);
		this->num_instances_until_target = until_distribution(generator);

		wrapper->new_since_update++;

		if (history->has_explore) {
			// temp
			if (this->state_iter == 0) {
				cout << "history->predicted: " << history->predicted << endl;
				cout << "target_val: " << target_val << endl;
				cout << "wrapper->iters_since_update: " << wrapper->iters_since_update << endl;
				cout << endl;
			}

			this->state_iter++;

			double curr_surprise = target_val - history->existing_predicted;

			bool is_success = false;
			if (curr_surprise >= 0.0) {
				if ((int)this->surprises.size() >= MIN_NUM_SAMPLES) {
					int index = BETTER_THAN_RATIO * (double)this->surprises.size();
					if (curr_surprise >= this->surprises[index]) {
						is_success = true;
					}
				}

				int index = 0;
				while (true) {
					if (index >= (int)this->surprises.size()) {
						break;
					}

					if (curr_surprise <= this->surprises[index]) {
						break;
					}

					index++;
				}

				this->surprises.insert(this->surprises.begin() + index, curr_surprise);
			}

			if (is_success) {
				// // temp
				// cout << "this->state_iter: " << this->state_iter << endl;

				this->best_step_types = history->curr_step_types;
				this->best_indexes = history->curr_indexes;

				this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_NEW;
				this->state_iter = 0;
			}
		}
	}
}
