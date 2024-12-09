#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "potential_commit.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void commit(Solution* parent_solution) {
	while (true) {
		run_index++;

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		vector<ContextLayer> commit_gather_context;
		int node_count = 0;
		AbstractNode* potential_node_context;
		bool potential_is_branch;
		parent_solution->scopes[0]->commit_gather_activate(
				problem,
				commit_gather_context,
				run_helper,
				node_count,
				potential_node_context,
				potential_is_branch);

		double existing_target_val = problem->score_result();
		existing_target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
		existing_target_val -= run_helper.num_analyze * solution->curr_time_penalty;

		/**
		 * - exit in-place to not delete existing nodes
		 */
		AbstractNode* exit_next_node;
		switch (potential_node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)potential_node_context;
				exit_next_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)potential_node_context;
				exit_next_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)potential_node_context;
				if (potential_is_branch) {
					exit_next_node = branch_node->branch_next_node;
				} else {
					exit_next_node = branch_node->original_next_node;
				}
			}
			break;
		}

		geometric_distribution<int> geo_distribution(0.2);
		int new_num_steps = 3 + geo_distribution(generator);

		vector<int> step_types;
		vector<ActionNode*> actions;
		vector<ScopeNode*> scopes;
		/**
		 * - always give raw actions a large weight
		 *   - existing scopes often learned to avoid certain patterns
		 *     - which can prevent innovation
		 */
		uniform_int_distribution<int> scope_distribution(0, 2);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (potential_node_context->parent->child_scopes.size() > 0
					&& scope_distribution(generator) == 0) {
				step_types.push_back(STEP_TYPE_SCOPE);
				actions.push_back(NULL);

				ScopeNode* new_scope_node = new ScopeNode();
				uniform_int_distribution<int> child_scope_distribution(0, potential_node_context->parent->child_scopes.size()-1);
				new_scope_node->scope = potential_node_context->parent->child_scopes[child_scope_distribution(generator)];
				scopes.push_back(new_scope_node);
			} else {
				step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				actions.push_back(new_action_node);

				scopes.push_back(NULL);
			}
		}

		PotentialCommit* potential_commit = new PotentialCommit();
		potential_commit->node_context = potential_node_context;
		potential_commit->is_branch = potential_is_branch;
		potential_commit->step_types = step_types;
		potential_commit->actions = actions;
		potential_commit->scopes = scopes;
		potential_commit->exit_next_node = exit_next_node;

		Problem* copy_problem = problem->copy_and_reset();

		run_helper.num_analyze = 0;
		run_helper.num_actions = 0;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		vector<ContextLayer> commit_context;
		solution->scopes[0]->commit_activate(
			copy_problem,
			commit_context,
			run_helper,
			potential_commit);

		double new_target_val = copy_problem->score_result();
		new_target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
		new_target_val -= run_helper.num_analyze * solution->curr_time_penalty;

		delete copy_problem;
		delete problem;

		if (new_target_val > existing_target_val) {
			potential_commit->finalize();

			delete potential_commit;

			break;
		} else {
			delete potential_commit;
		}
	}
}
