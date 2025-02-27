#include "branch_experiment.h"

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
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 5;
#else
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 500;
#endif /* MDEBUG */

void BranchExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	run_helper.num_actions++;

	this->num_instances_until_target--;
	if (history->existing_predicted_scores.size() == 0
			&& this->num_instances_until_target == 0) {
		double sum_vals = this->existing_average_score;
		for (int f_index = 0; f_index < (int)this->existing_factors.size(); f_index++) {
			double val;
			fetch_input(run_helper,
						scope_history,
						this->existing_factors[f_index],
						val);
			sum_vals += this->existing_factor_weights[f_index] * val;
		}
		history->existing_predicted_scores.push_back(sum_vals);

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

		if (possible_exits.size() < 10) {
			uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
			this->curr_exit_next_node = possible_exits[exit_distribution(generator)];
		} else {
			geometric_distribution<int> exit_distribution(0.2);
			int random_index = exit_distribution(generator);
			if (random_index >= (int)possible_exits.size()) {
				random_index = (int)possible_exits.size()-1;
			}
			this->curr_exit_next_node = possible_exits[random_index];
		}

		geometric_distribution<int> geo_distribution(0.3);
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
				this->curr_step_types.push_back(STEP_TYPE_SCOPE);
				this->curr_actions.push_back(Action());

				uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
				this->curr_scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
			} else if (type >= 1 && solution->existing_scopes.size() > 0
					&& (scope_history->scope->id == 0 || scope_history->scope->id > (int)solution->existing_scopes.size())) {
				this->curr_step_types.push_back(STEP_TYPE_SCOPE);
				this->curr_actions.push_back(Action());

				uniform_int_distribution<int> existing_scope_distribution(0, solution->existing_scopes.size()-1);
				this->curr_scopes.push_back(solution->existing_scopes[existing_scope_distribution(generator)]);
			} else {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
				this->curr_actions.push_back(Action(action_distribution(generator)));

				this->curr_scopes.push_back(NULL);
			}
		}

		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->curr_actions[s_index]);
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[s_index]);

				inner_scope_history->input_history = vector<double>(this->curr_scopes[s_index]->num_inputs, 0.0);
				map<Scope*, vector<Input>>::iterator it = this->scope_context->child_scope_inputs.find(this->curr_scopes[s_index]);
				if (it != this->scope_context->child_scope_inputs.end()) {
					for (int i_index = 0; i_index < this->curr_scopes[s_index]->num_inputs; i_index++) {
						fetch_input(run_helper,
									scope_history,
									it->second[i_index],
									inner_scope_history->input_history[i_index]);
					}
				}

				this->curr_scopes[s_index]->activate(problem,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;
			}

			run_helper.num_actions += 2;
		}

		curr_node = this->curr_exit_next_node;
	}
}

void BranchExperiment::explore_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_history;

	uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1);
	this->num_instances_until_target = 1 + until_distribution(generator);

	if (history->existing_predicted_scores.size() > 0) {
		double curr_surprise = target_val - history->existing_predicted_scores[0];
		#if defined(MDEBUG) && MDEBUG
		if (true) {
		#else
		if (curr_surprise > this->best_surprise) {
		#endif /* MDEBUG */
			this->best_surprise = curr_surprise;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_scopes = this->curr_scopes;
			this->best_exit_next_node = this->curr_exit_next_node;

			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_scopes.clear();
		} else {
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_scopes.clear();
		}

		this->state_iter++;
		if (this->state_iter >= BRANCH_EXPERIMENT_EXPLORE_ITERS) {
			if (this->best_surprise > 0.0) {
				this->state = BRANCH_EXPERIMENT_STATE_NEW_GATHER;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
