#include "commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 5;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 10;
const int STEP_TRY_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 100;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 500;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 2000;
const int STEP_TRY_ITERS = 100;
#endif /* MDEBUG */

void CommitExperiment::find_save_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper) {
	run_helper.has_explore = true;

	for (int s_index = 0; s_index < this->step_iter; s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			problem->perform_action(this->best_actions[s_index]);
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
			this->best_scopes[s_index]->activate(problem,
				run_helper,
				inner_scope_history);
			delete inner_scope_history;
		}

		run_helper.num_actions += 2;
	}

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

		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
		int random_index = distribution(generator);
		this->save_exit_next_node = possible_exits[random_index];

		geometric_distribution<int> geo_distribution(0.2);
		int new_num_steps = geo_distribution(generator);

		/**
		 * - always give raw actions a large weight
		 *   - existing scopes often learned to avoid certain patterns
		 *     - which can prevent innovation
		 */
		uniform_int_distribution<int> type_distribution(0, 2);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			int type = type_distribution(generator);
			if (type >= 2 && this->scope_context->child_scopes.size() > 0) {
				this->save_step_types.push_back(STEP_TYPE_SCOPE);
				this->save_actions.push_back(Action());

				uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
				this->save_scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
			} else if (type >= 1 && solution->existing_scopes.size() > 0) {
				this->save_step_types.push_back(STEP_TYPE_SCOPE);
				this->save_actions.push_back(Action());

				uniform_int_distribution<int> existing_scope_distribution(0, solution->existing_scopes.size()-1);
				this->save_scopes.push_back(solution->existing_scopes[existing_scope_distribution(generator)]);
			} else {
				this->save_step_types.push_back(STEP_TYPE_ACTION);

				this->save_actions.push_back(problem_type->random_action());

				this->save_scopes.push_back(NULL);
			}
		}

		this->state_iter = 0;
	}

	for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
		if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
			problem->perform_action(this->save_actions[s_index]);
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[s_index]);
			this->save_scopes[s_index]->activate(problem,
				run_helper,
				inner_scope_history);
			delete inner_scope_history;
		}

		run_helper.num_actions += 2;
	}

	curr_node = this->save_exit_next_node;
}

void CommitExperiment::find_save_backprop(
		double target_val,
		RunHelper& run_helper) {
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

				new_action_node->average_instances_per_run = this->node_context->average_instances_per_run;

				this->new_nodes.push_back(new_action_node);
			} else {
				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->parent = this->scope_context;
				new_scope_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				new_scope_node->scope = this->best_scopes[s_index];

				new_scope_node->average_instances_per_run = this->node_context->average_instances_per_run;

				this->new_nodes.push_back(new_scope_node);
			}

			ObsNode* new_obs_node = new ObsNode();
			new_obs_node->parent = this->scope_context;
			new_obs_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;

			new_obs_node->average_instances_per_run = node_context->average_instances_per_run;

			this->new_nodes.push_back(new_obs_node);
		}

		this->step_iter *= 2;

		uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);

		this->state = COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER;
		this->state_iter = 0;
	}
}
