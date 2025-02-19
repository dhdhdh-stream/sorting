#include "branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 20;
#else
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 500;
#endif /* MDEBUG */

void BranchExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentOverallHistory* overall_history) {
	this->num_instances_until_target--;
	if (!overall_history->has_target
			&& this->num_instances_until_target == 0) {
		overall_history->has_target = true;
		BranchExperimentInstanceHistory* instance_history = new BranchExperimentInstanceHistory(this);
		run_helper.instance_histories.push_back(instance_history);

		double sum_vals = this->existing_average_score;
		for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
			double val;
			fetch_factor_helper(scope_history,
								this->existing_factor_ids[f_index],
								val);
			sum_vals += this->existing_factor_weights[f_index] * val;
		}
		instance_history->existing_predicted_score = sum_vals;

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
		instance_history->curr_exit_next_node = possible_exits[random_index];

		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.3);
		if (random_index == 0) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}

		/**
		 * - always give raw actions a large weight
		 *   - existing scopes often learned to avoid certain patterns
		 *     - which can prevent innovation
		 */
		uniform_int_distribution<int> type_distribution(0, 2);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			int type = type_distribution(generator);
			if (type >= 2 && this->scope_context->child_scopes.size() > 0) {
				instance_history->curr_step_types.push_back(STEP_TYPE_SCOPE);
				instance_history->curr_actions.push_back(Action());

				uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
				instance_history->curr_scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
			} else if (type >= 1 && this->scope_context->existing_scopes.size() > 0) {
				instance_history->curr_step_types.push_back(STEP_TYPE_SCOPE);
				instance_history->curr_actions.push_back(Action());

				uniform_int_distribution<int> existing_scope_distribution(0, this->scope_context->existing_scopes.size()-1);
				instance_history->curr_scopes.push_back(this->scope_context->existing_scopes[existing_scope_distribution(generator)]);
			} else {
				instance_history->curr_step_types.push_back(STEP_TYPE_ACTION);

				instance_history->curr_actions.push_back(problem_type->random_action());

				instance_history->curr_scopes.push_back(NULL);
			}
		}

		for (int s_index = 0; s_index < (int)instance_history->curr_step_types.size(); s_index++) {
			if (instance_history->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(instance_history->curr_actions[s_index]);
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(instance_history->curr_scopes[s_index]);
				instance_history->curr_scopes[s_index]->activate(problem,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;
			}

			run_helper.num_actions += 2;
		}

		curr_node = instance_history->curr_exit_next_node;
	}
}

void BranchExperiment::explore_backprop(
		BranchExperimentInstanceHistory* instance_history,
		double target_val) {
	double curr_surprise = target_val - instance_history->existing_predicted_score;
	if (curr_surprise > this->surprises.back()) {
		this->surprises.back() = curr_surprise;
		this->step_types.back() = instance_history->curr_step_types;
		this->actions.back() = instance_history->curr_actions;
		this->scopes.back() = instance_history->curr_scopes;
		this->exit_next_node.back() = instance_history->curr_exit_next_node;

		int curr_index = BRANCH_EXPERIMENT_NUM_CONCURRENT-1;
		while (true) {
			if (curr_index == 0) {
				break;
			}

			if (this->surprises[curr_index] > this->surprises[curr_index-1]) {
				double temp_surprise = this->surprises[curr_index];
				vector<int> temp_step_types = this->step_types[curr_index];
				vector<Action> temp_actions = this->actions[curr_index];
				vector<Scope*> temp_scopes = this->scopes[curr_index];
				AbstractNode* temp_exit_next_node = this->exit_next_node[curr_index];

				this->surprises[curr_index] = this->surprises[curr_index-1];
				this->step_types[curr_index] = this->step_types[curr_index-1];
				this->actions[curr_index] = this->actions[curr_index-1];
				this->scopes[curr_index] = this->scopes[curr_index-1];
				this->exit_next_node[curr_index] = this->exit_next_node[curr_index-1];

				this->surprises[curr_index-1] = temp_surprise;
				this->step_types[curr_index-1] = temp_step_types;
				this->actions[curr_index-1] = temp_actions;
				this->scopes[curr_index-1] = temp_scopes;
				this->exit_next_node[curr_index-1] = temp_exit_next_node;

				curr_index--;
			} else {
				break;
			}
		}
	}

	this->instance_iter++;
}

void BranchExperiment::explore_update() {
	uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1);
	this->num_instances_until_target = 1 + until_distribution(generator);

	if (this->instance_iter >= BRANCH_EXPERIMENT_EXPLORE_ITERS) {
		if (this->surprises.back() > 0.0) {
			this->new_inputs = vector<vector<pair<pair<vector<Scope*>,vector<int>>,pair<int,int>>>>(BRANCH_EXPERIMENT_NUM_CONCURRENT);
			this->new_factor_ids = vector<vector<pair<int,int>>>(BRANCH_EXPERIMENT_NUM_CONCURRENT);

			this->state = BRANCH_EXPERIMENT_STATE_NEW_GATHER;
			this->instance_iter = 0;
			this->run_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
