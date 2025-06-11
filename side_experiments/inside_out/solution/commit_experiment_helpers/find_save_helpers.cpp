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

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 5;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 10;
const int STEP_TRY_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 40;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 400;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 4000;
const int STEP_TRY_ITERS = 100;
#endif /* MDEBUG */

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
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (scope_distribution(generator) == 0 && this->scope_context->child_scopes.size() > 0) {
				this->save_step_types.push_back(STEP_TYPE_SCOPE);
				this->save_actions.push_back(-1);

				uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
				this->save_scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
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
	bool is_fail = false;

	this->save_sum_score += target_val;

	this->state_iter++;
	if (this->state_iter == INITIAL_NUM_SAMPLES_PER_ITER
			|| this->state_iter == VERIFY_1ST_NUM_SAMPLES_PER_ITER
			|| this->state_iter == VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
		#if defined(MDEBUG) && MDEBUG
		if (false) {
		#else
		double curr_score = this->save_sum_score / this->state_iter;
		if (curr_score <= this->o_existing_average_score) {
		#endif /* MDEBUG */
			is_fail = true;
		}
	}

	if (is_fail) {
		this->save_sum_score = 0.0;
		this->save_step_types.clear();
		this->save_actions.clear();
		this->save_scopes.clear();

		this->state_iter = -1;

		this->save_iter++;
		if (this->save_iter >= STEP_TRY_ITERS) {
			this->save_iter = 0;

			this->step_iter--;
			if (this->step_iter == 0) {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	} else if (this->state_iter >= VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
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

		this->state = COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER;
		this->state_iter = 0;
	}
}
