// #include "commit_experiment.h"

// #include <iostream>

// #include "action_node.h"
// #include "branch_experiment.h"
// #include "branch_node.h"
// #include "constants.h"
// #include "globals.h"
// #include "new_scope_experiment.h"
// #include "obs_node.h"
// #include "problem.h"
// #include "scope.h"
// #include "scope_node.h"
// #include "solution.h"
// #include "solution_helpers.h"
// #include "solution_wrapper.h"

// using namespace std;

// #if defined(MDEBUG) && MDEBUG
// const int COMMIT_EXPERIMENT_EXPLORE_ITERS = 5;
// #else
// const int COMMIT_EXPERIMENT_EXPLORE_ITERS = 100;
// #endif /* MDEBUG */

// void CommitExperiment::explore_check_activate(
// 		SolutionWrapper* wrapper,
// 		CommitExperimentHistory* history) {
// 	this->num_instances_until_target--;
// 	if (history->existing_predicted_scores.size() == 0
// 			&& this->num_instances_until_target == 0) {
// 		double sum_vals = this->existing_average_score;
// 		for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
// 			double val;
// 			bool is_on;
// 			fetch_input_helper(wrapper->scope_histories.back(),
// 							   this->existing_inputs[i_index],
// 							   0,
// 							   val,
// 							   is_on);
// 			if (is_on) {
// 				double normalized_val = (val - this->existing_input_averages[i_index]) / this->existing_input_standard_deviations[i_index];
// 				sum_vals += this->existing_weights[i_index] * normalized_val;
// 			}
// 		}
// 		history->existing_predicted_scores.push_back(sum_vals);

// 		/**
// 		 * - exit in-place to not delete existing nodes
// 		 */
// 		switch (this->node_context->type) {
// 		case NODE_TYPE_ACTION:
// 			{
// 				ActionNode* action_node = (ActionNode*)this->node_context;
// 				this->curr_exit_next_node = action_node->next_node;
// 			}
// 			break;
// 		case NODE_TYPE_SCOPE:
// 			{
// 				ScopeNode* scope_node = (ScopeNode*)this->node_context;
// 				this->curr_exit_next_node = scope_node->next_node;
// 			}
// 			break;
// 		case NODE_TYPE_BRANCH:
// 			{
// 				BranchNode* branch_node = (BranchNode*)this->node_context;
// 				if (this->is_branch) {
// 					this->curr_exit_next_node = branch_node->branch_next_node;
// 				} else {
// 					this->curr_exit_next_node = branch_node->original_next_node;
// 				}
// 			}
// 			break;
// 		case NODE_TYPE_OBS:
// 			{
// 				ObsNode* obs_node = (ObsNode*)this->node_context;
// 				this->curr_exit_next_node = obs_node->next_node;
// 			}
// 			break;
// 		}

// 		geometric_distribution<int> geo_distribution(0.2);
// 		int new_num_steps = 3 + geo_distribution(generator);

// 		/**
// 		 * - always give raw actions a large weight
// 		 *   - existing scopes often learned to avoid certain patterns
// 		 *     - which can prevent innovation
// 		 */
// 		uniform_int_distribution<int> scope_distribution(0, 1);
// 		vector<Scope*> possible_scopes;
// 		for (int c_index = 0; c_index < (int)this->scope_context->child_scopes.size(); c_index++) {
// 			if (this->scope_context->child_scopes[c_index]->nodes.size() > 1) {
// 				possible_scopes.push_back(this->scope_context->child_scopes[c_index]);
// 			}
// 		}
// 		for (int s_index = 0; s_index < new_num_steps; s_index++) {
// 			if (scope_distribution(generator) == 0 && possible_scopes.size() > 0) {
// 				this->curr_step_types.push_back(STEP_TYPE_SCOPE);
// 				this->curr_actions.push_back(-1);

// 				uniform_int_distribution<int> child_scope_distribution(0, possible_scopes.size()-1);
// 				this->curr_scopes.push_back(possible_scopes[child_scope_distribution(generator)]);
// 			} else {
// 				this->curr_step_types.push_back(STEP_TYPE_ACTION);

// 				this->curr_actions.push_back(-1);

// 				this->curr_scopes.push_back(NULL);
// 			}
// 		}

// 		CommitExperimentState* new_experiment_state = new CommitExperimentState(this);
// 		new_experiment_state->is_save = false;
// 		new_experiment_state->step_index = 0;
// 		wrapper->experiment_context.back() = new_experiment_state;
// 	}
// }

// void CommitExperiment::explore_step(vector<double>& obs,
// 									int& action,
// 									bool& is_next,
// 									bool& fetch_action,
// 									SolutionWrapper* wrapper,
// 									CommitExperimentState* experiment_state) {
// 	if (experiment_state->step_index >= (int)this->curr_step_types.size()) {
// 		wrapper->node_context.back() = this->curr_exit_next_node;

// 		delete experiment_state;
// 		wrapper->experiment_context.back() = NULL;
// 	} else {
// 		if (this->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
// 			is_next = true;
// 			fetch_action = true;

// 			wrapper->num_actions++;
// 		} else {
// 			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[experiment_state->step_index]);
// 			wrapper->scope_histories.push_back(inner_scope_history);
// 			wrapper->node_context.push_back(this->curr_scopes[experiment_state->step_index]->nodes[0]);
// 			wrapper->experiment_context.push_back(NULL);
// 		}
// 	}
// }

// void CommitExperiment::explore_set_action(int action,
// 										  CommitExperimentState* experiment_state) {
// 	this->curr_actions[experiment_state->step_index] = action;

// 	experiment_state->step_index++;
// }

// void CommitExperiment::explore_exit_step(SolutionWrapper* wrapper,
// 										 CommitExperimentState* experiment_state) {
// 	if (this->curr_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
// 		this->curr_scopes[experiment_state->step_index]->new_scope_experiment->back_activate(wrapper);
// 	}

// 	delete wrapper->scope_histories.back();

// 	wrapper->scope_histories.pop_back();
// 	wrapper->node_context.pop_back();
// 	wrapper->experiment_context.pop_back();

// 	experiment_state->step_index++;
// }

// void CommitExperiment::explore_backprop(
// 		double target_val,
// 		CommitExperimentHistory* history) {
// 	if (history->is_hit) {
// 		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1);
// 		this->num_instances_until_target = 1 + until_distribution(generator);

// 		if (history->existing_predicted_scores.size() > 0) {
// 			double curr_surprise = target_val - history->existing_predicted_scores[0];

// 			#if defined(MDEBUG) && MDEBUG
// 			if (true) {
// 			#else
// 			if (curr_surprise > this->best_surprise) {
// 			#endif /* MDEBUG */
// 				this->best_surprise = curr_surprise;
// 				this->best_step_types = this->curr_step_types;
// 				this->best_actions = this->curr_actions;
// 				this->best_scopes = this->curr_scopes;
// 				this->best_exit_next_node = this->curr_exit_next_node;

// 				this->curr_step_types.clear();
// 				this->curr_actions.clear();
// 				this->curr_scopes.clear();
// 			} else {
// 				this->curr_step_types.clear();
// 				this->curr_actions.clear();
// 				this->curr_scopes.clear();
// 			}

// 			this->state_iter++;
// 			if (this->state_iter >= COMMIT_EXPERIMENT_EXPLORE_ITERS) {
// 				if (this->best_surprise > 0.0) {
// 					this->step_iter = (int)this->best_step_types.size();
// 					this->save_iter = 0;

// 					this->state_iter = -1;

// 					this->state = COMMIT_EXPERIMENT_STATE_FIND_SAVE;
// 				} else {
// 					this->result = EXPERIMENT_RESULT_FAIL;
// 				}
// 			}
// 		}
// 	}
// }
