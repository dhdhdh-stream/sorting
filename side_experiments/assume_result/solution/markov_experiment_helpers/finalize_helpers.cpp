#include "markov_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "markov_node.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void MarkovExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];
		AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];

		MarkovNode* new_node = new MarkovNode();
		new_node->parent = duplicate_local_scope;
		new_node->id = duplicate_local_scope->node_counter;
		duplicate_local_scope->node_counter++;
		duplicate_local_scope->nodes[new_node->id] = new_node;

		new_node->step_types = this->step_types;
		new_node->actions = this->actions;
		new_node->scopes = this->scopes;
		for (int o_index = 0; o_index < (int)new_node->scopes.size(); o_index++) {
			for (int s_index = 0; s_index < (int)new_node->scopes[o_index].size(); s_index++) {
				if (new_node->scopes[o_index][s_index] != NULL) {
					new_node->scopes[o_index][s_index] = duplicate->scopes[new_node->scopes[o_index][s_index]->id];
				}
			}
		}
		new_node->networks = this->networks;
		this->networks.clear();
		new_node->halt_network = this->halt_network;
		this->halt_network = NULL;

		if (this->exit_next_node == NULL) {
			ActionNode* new_ending_node = new ActionNode();
			new_ending_node->parent = duplicate_local_scope;
			new_ending_node->id = duplicate_local_scope->node_counter;
			duplicate_local_scope->node_counter++;
			duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

			new_ending_node->action = Action(ACTION_NOOP);

			new_ending_node->next_node_id = -1;
			new_ending_node->next_node = NULL;

			new_node->next_node_id = new_ending_node->id;
			new_node->next_node = new_ending_node;
		} else {
			new_node->next_node_id = this->exit_next_node->id;
			new_node->next_node = duplicate_local_scope->nodes[this->exit_next_node->id];
		}

		switch (duplicate_explore_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)duplicate_explore_node;

				action_node->next_node_id = new_node->id;
				action_node->next_node = new_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

				scope_node->next_node_id = new_node->id;
				scope_node->next_node = new_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

				if (this->is_branch) {
					branch_node->branch_next_node_id = new_node->id;
					branch_node->branch_next_node = new_node;
				} else {
					branch_node->original_next_node_id = new_node->id;
					branch_node->original_next_node = new_node;
				}
			}
			break;
		case NODE_TYPE_RETURN:
			{
				ReturnNode* return_node = (ReturnNode*)duplicate_explore_node;

				if (this->is_branch) {
					return_node->passed_next_node_id = new_node->id;
					return_node->passed_next_node = new_node;
				} else {
					return_node->skipped_next_node_id = new_node->id;
					return_node->skipped_next_node = new_node;
				}
			}
			break;
		case NODE_TYPE_MARKOV:
			{
				MarkovNode* markov_node = (MarkovNode*)duplicate_explore_node;

				markov_node->next_node_id = new_node->id;
				markov_node->next_node = new_node;
			}
			break;
		}
	}

	int experiment_index;
	for (int e_index = 0; e_index < (int)this->node_context->experiments.size(); e_index++) {
		if (this->node_context->experiments[e_index] == this) {
			experiment_index = e_index;
			break;
		}
	}
	this->node_context->experiments.erase(this->node_context->experiments.begin() + experiment_index);
}
