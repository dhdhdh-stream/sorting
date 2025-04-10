#include "pass_through_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_EXISTING_NUM_DATAPOINTS = 20;
#else
const int MEASURE_EXISTING_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void PassThroughExperiment::measure_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->sum_score += target_val;

	this->state_iter++;
	if (this->state_iter >= MEASURE_EXISTING_NUM_DATAPOINTS) {
		this->existing_average_score = this->sum_score / this->state_iter;

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
		if (this->scope_context->exceeded) {
			if (possible_exits.size() <= 4) {
				this->result = EXPERIMENT_RESULT_FAIL;
				return;
			} else {
				uniform_int_distribution<int> distribution(4, possible_exits.size()-1);
				random_index = distribution(generator);
			}
		} else {
			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			random_index = distribution(generator);
		}
		this->exit_next_node = possible_exits[random_index];

		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.2);
		if (random_index == 0) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}
		if (this->scope_context->exceeded) {
			if (new_num_steps > random_index/2-1) {
				new_num_steps = random_index/2-1;
			}
		}

		/**
		 * - always give raw actions a large weight
		 *   - existing scopes often learned to avoid certain patterns
		 *     - which can prevent innovation
		 */
		uniform_int_distribution<int> scope_distribution(0, 1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (scope_distribution(generator) == 0 && this->scope_context->child_scopes.size() > 0) {
				this->step_types.push_back(STEP_TYPE_SCOPE);
				this->actions.push_back(Action());

				uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
				this->scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
			} else {
				this->step_types.push_back(STEP_TYPE_ACTION);

				this->actions.push_back(problem_type->random_action());

				this->scopes.push_back(NULL);
			}
		}

		this->sum_score = 0.0;

		this->state = PASS_THROUGH_EXPERIMENT_STATE_INITIAL;
		this->state_iter = 0;
		this->explore_iter = 0;
	}
}
