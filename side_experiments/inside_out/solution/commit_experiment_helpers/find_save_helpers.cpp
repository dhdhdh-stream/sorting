#include "commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const int STEP_TRY_ITERS = 100;

void CommitExperiment::find_save_check_activate(
		SolutionWrapper* wrapper) {
	if (this->state_iter == -1) {
		vector<AbstractNode*> possible_exits;

		AbstractNode* starting_node;
		switch (this->node_context->type) {
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

		int random_index;
		if (possible_exits.size() < 20) {
			uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
			random_index = exit_distribution(generator);
		} else {
			geometric_distribution<int> exit_distribution(0.1);
			random_index = exit_distribution(generator);
			if (random_index >= (int)possible_exits.size()) {
				random_index = (int)possible_exits.size()-1;
			}
		}
		this->save_exit_next_node = possible_exits[random_index];

		geometric_distribution<int> geo_distribution(0.2);
		int new_num_steps = geo_distribution(generator);

		/**
		 * - always give raw actions a large weight
		 *   - existing scopes often learned to avoid certain patterns
		 *     - which can prevent innovation
		 */
		uniform_int_distribution<int> scope_distribution(0, 1);
		vector<Scope*> possible_scopes;
		for (int c_index = 0; c_index < (int)this->scope_context->child_scopes.size(); c_index++) {
			if (this->scope_context->child_scopes[c_index]->nodes.size() > 1) {
				possible_scopes.push_back(this->scope_context->child_scopes[c_index]);
			}
		}
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (scope_distribution(generator) == 0 && possible_scopes.size() > 0) {
				this->save_step_types.push_back(STEP_TYPE_SCOPE);
				this->save_actions.push_back(-1);

				uniform_int_distribution<int> child_scope_distribution(0, possible_scopes.size()-1);
				this->save_scopes.push_back(possible_scopes[child_scope_distribution(generator)]);
			} else {
				this->save_step_types.push_back(STEP_TYPE_ACTION);

				this->save_actions.push_back(-1);

				this->save_scopes.push_back(NULL);
			}
		}
		this->save_is_init = false;

		this->state_iter = 0;
	}

	CommitExperimentState* new_experiment_state = new CommitExperimentState(this);
	new_experiment_state->is_save = false;
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void CommitExperiment::find_save_step(vector<double>& obs,
									  int& action,
									  bool& is_next,
									  bool& fetch_action,
									  SolutionWrapper* wrapper,
									  CommitExperimentState* experiment_state) {
	if (experiment_state->is_save) {
		if (experiment_state->step_index >= (int)this->save_step_types.size()) {
			wrapper->node_context.back() = this->save_exit_next_node;

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;

			this->save_is_init = true;
		} else {
			if (this->save_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
				if (this->save_is_init) {
					action = this->save_actions[experiment_state->step_index];
					is_next = true;

					wrapper->num_actions++;

					experiment_state->step_index++;
				} else {
					is_next = true;
					fetch_action = true;

					wrapper->num_actions++;
				}
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[experiment_state->step_index]);
				wrapper->scope_histories.push_back(inner_scope_history);
				wrapper->node_context.push_back(this->save_scopes[experiment_state->step_index]->nodes[0]);
				wrapper->experiment_context.push_back(NULL);
				wrapper->confusion_context.push_back(NULL);

				if (this->save_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
					this->save_scopes[experiment_state->step_index]->new_scope_experiment->pre_activate(wrapper);
				}
			}
		}
	} else {
		if (experiment_state->step_index >= this->step_iter) {
			experiment_state->is_save = true;
			experiment_state->step_index = 0;
		} else {
			if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
				action = this->best_actions[experiment_state->step_index];
				is_next = true;

				wrapper->num_actions++;

				experiment_state->step_index++;
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
				wrapper->scope_histories.push_back(inner_scope_history);
				wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
				wrapper->experiment_context.push_back(NULL);
				wrapper->confusion_context.push_back(NULL);

				if (this->best_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
					this->best_scopes[experiment_state->step_index]->new_scope_experiment->pre_activate(wrapper);
				}
			}
		}
	}
}

void CommitExperiment::find_save_set_action(int action,
											CommitExperimentState* experiment_state) {
	this->save_actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void CommitExperiment::find_save_exit_step(SolutionWrapper* wrapper,
										   CommitExperimentState* experiment_state) {
	if (experiment_state->is_save) {
		if (this->save_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
			this->save_scopes[experiment_state->step_index]->new_scope_experiment->back_activate(wrapper);
		}

		delete wrapper->scope_histories.back();

		wrapper->scope_histories.pop_back();
		wrapper->node_context.pop_back();
		wrapper->experiment_context.pop_back();
		wrapper->confusion_context.pop_back();

		experiment_state->step_index++;
	} else {
		if (this->best_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
			this->best_scopes[experiment_state->step_index]->new_scope_experiment->back_activate(wrapper);
		}

		delete wrapper->scope_histories.back();

		wrapper->scope_histories.pop_back();
		wrapper->node_context.pop_back();
		wrapper->experiment_context.pop_back();
		wrapper->confusion_context.pop_back();

		experiment_state->step_index++;
	}
}

void CommitExperiment::find_save_backprop(
		double target_val) {
	this->i_target_val_histories.push_back(target_val);

	this->state_iter++;
	if (this->state_iter == EARLY_SUCCESS_S1_ITERS
			|| this->state_iter == EARLY_SUCCESS_S2_ITERS) {
		double sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->i_target_val_histories.size(); h_index++) {
			sum_score += this->i_target_val_histories[h_index];
		}
		double new_score = sum_score / (double)this->state_iter;

		double sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->i_target_val_histories.size(); h_index++) {
			sum_variance += (this->i_target_val_histories[h_index] - new_score)
				* (this->i_target_val_histories[h_index] - new_score);
		}
		double new_standard_deviation = sqrt(sum_variance / (double)this->i_target_val_histories.size());
		if (new_standard_deviation < MIN_STANDARD_DEVIATION) {
			new_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		double existing_score;
		switch (this->node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				existing_score = action_node->average_score;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				existing_score = scope_node->average_score;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					existing_score = branch_node->branch_average_score;
				} else {
					existing_score = branch_node->original_average_score;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;
				existing_score = obs_node->average_score;
			}
			break;
		}

		double t_score = (new_score - existing_score) / new_standard_deviation;
		if (t_score >= EARLY_SUCCESS_MIN_T_SCORE) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNode* new_action_node = new ActionNode();
					new_action_node->parent = this->scope_context;
					new_action_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_action_node->action = this->best_actions[s_index];

					this->new_nodes.push_back(new_action_node);
				} else {
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->parent = this->scope_context;
					new_scope_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_scope_node->scope = this->best_scopes[s_index];

					this->new_nodes.push_back(new_scope_node);
				}

				ObsNode* new_obs_node = new ObsNode();
				new_obs_node->parent = this->scope_context;
				new_obs_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				this->new_nodes.push_back(new_obs_node);
			}

			this->step_iter *= 2;

			uniform_int_distribution<int> until_distribution(0, 2*((int)this->average_instances_per_run-1));
			this->num_instances_until_target = 1 + until_distribution(generator);

			this->state = COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING;
			this->state_iter = 0;
		}
	} else if (this->state_iter == MEASURE_S1_ITERS
			|| this->state_iter == MEASURE_S2_ITERS
			|| this->state_iter == MEASURE_S3_ITERS
			|| this->state_iter == MEASURE_S4_ITERS) {
		#if defined(MDEBUG) && MDEBUG
		if (false) {
		#else
		double sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->i_target_val_histories.size(); h_index++) {
			sum_score += this->i_target_val_histories[h_index];
		}
		double new_score = sum_score / (double)this->state_iter;

		double existing_score;
		switch (this->node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				existing_score = action_node->average_score;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				existing_score = scope_node->average_score;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					existing_score = branch_node->branch_average_score;
				} else {
					existing_score = branch_node->original_average_score;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;
				existing_score = obs_node->average_score;
			}
			break;
		}

		if (new_score < existing_score) {
		#endif /* MDEBUG */
			this->save_step_types.clear();
			this->save_actions.clear();
			this->save_scopes.clear();

			this->i_target_val_histories.clear();

			this->state_iter = -1;

			this->save_iter++;
			if (this->save_iter >= STEP_TRY_ITERS) {
				this->save_iter = 0;

				this->step_iter--;
				if (this->step_iter == 0) {
					this->result = EXPERIMENT_RESULT_FAIL;
				}
			}
		} else if (this->state_iter >= MEASURE_S4_ITERS) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNode* new_action_node = new ActionNode();
					new_action_node->parent = this->scope_context;
					new_action_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_action_node->action = this->best_actions[s_index];

					this->new_nodes.push_back(new_action_node);
				} else {
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->parent = this->scope_context;
					new_scope_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_scope_node->scope = this->best_scopes[s_index];

					this->new_nodes.push_back(new_scope_node);
				}

				ObsNode* new_obs_node = new ObsNode();
				new_obs_node->parent = this->scope_context;
				new_obs_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				this->new_nodes.push_back(new_obs_node);
			}

			this->step_iter *= 2;

			uniform_int_distribution<int> until_distribution(0, 2*((int)this->average_instances_per_run-1));
			this->num_instances_until_target = 1 + until_distribution(generator);

			this->state = COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING;
			this->state_iter = 0;
		}
	}
}
