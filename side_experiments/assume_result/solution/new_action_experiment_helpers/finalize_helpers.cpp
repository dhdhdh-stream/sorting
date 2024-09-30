#include "new_action_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "markov_node.h"
#include "network.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void NewActionExperiment::finalize(Solution* duplicate) {
	Scope* parent_scope = (Scope*)this->scope_context;
	parent_scope->new_action_experiment = NULL;

	for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
		int experiment_index;
		for (int e_index = 0; e_index < (int)this->test_location_starts[t_index]->experiments.size(); e_index++) {
			if (this->test_location_starts[t_index]->experiments[e_index] == this) {
				experiment_index = e_index;
				break;
			}
		}
		this->test_location_starts[t_index]->experiments.erase(this->test_location_starts[t_index]->experiments.begin() + experiment_index);
	}
	for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
		int experiment_index;
		for (int e_index = 0; e_index < (int)this->successful_location_starts[s_index]->experiments.size(); e_index++) {
			if (this->successful_location_starts[s_index]->experiments[e_index] == this) {
				experiment_index = e_index;
				break;
			}
		}
		this->successful_location_starts[s_index]->experiments.erase(this->successful_location_starts[s_index]->experiments.begin() + experiment_index);
	}

	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		cout << "NewActionExperiment success" << endl;

		this->new_scope->id = duplicate->scopes.size();
		duplicate->scopes.push_back(this->new_scope);

		for (map<int, AbstractNode*>::iterator it = this->new_scope->nodes.begin();
				it != this->new_scope->nodes.end(); it++) {
			switch (it->second->type) {
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)it->second;
					scope_node->scope = duplicate->scopes[scope_node->scope->id];
				}
				break;
			#if defined(MDEBUG) && MDEBUG
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)it->second;
					branch_node->verify_key = this;
				}
				break;
			#endif /* MDEBUG */
			case NODE_TYPE_MARKOV:
				{
					MarkovNode* markov_node = (MarkovNode*)it->second;
					for (int o_index = 0; o_index < (int)markov_node->scopes.size(); o_index++) {
						for (int s_index = 0; s_index < (int)markov_node->scopes[o_index].size(); s_index++) {
							if (markov_node->scopes[o_index][s_index] != NULL) {
								markov_node->scopes[o_index][s_index] = duplicate->scopes[markov_node->scopes[o_index][s_index]->id];
							}
						}
					}
				}
				break;
			}
		}

		#if defined(MDEBUG) && MDEBUG
		duplicate->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		duplicate->verify_seeds = this->verify_seeds;
		#endif /* MDEBUG */

		ActionNode* new_local_ending_node = NULL;

		Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];

		for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
			ScopeNode* new_scope_node = this->successful_scope_nodes[s_index];
			new_scope_node->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[new_scope_node->id] = new_scope_node;

			if (new_scope_node->next_node == NULL) {
				if (new_local_ending_node == NULL) {
					new_local_ending_node = new ActionNode();
					new_local_ending_node->parent = duplicate_local_scope;
					new_local_ending_node->id = duplicate_local_scope->node_counter;
					duplicate_local_scope->node_counter++;
					duplicate_local_scope->nodes[new_local_ending_node->id] = new_local_ending_node;

					new_local_ending_node->action = Action(ACTION_NOOP);

					new_local_ending_node->next_node_id = -1;
					new_local_ending_node->next_node = NULL;

					for (map<int, AbstractNode*>::iterator it = duplicate_local_scope->nodes.begin();
							it != duplicate_local_scope->nodes.end(); it++) {
						if (it->second->type == NODE_TYPE_ACTION) {
							ActionNode* action_node = (ActionNode*)it->second;
							if (action_node->next_node == NULL
									&& action_node != new_local_ending_node) {
								action_node->next_node_id = new_local_ending_node->id;
								action_node->next_node = new_local_ending_node;
							}
						}
					}
				}

				new_scope_node->next_node_id = new_local_ending_node->id;
				new_scope_node->next_node = new_local_ending_node;
			} else {
				AbstractNode* duplicate_end_node = duplicate_local_scope->nodes[new_scope_node->next_node->id];

				new_scope_node->next_node_id = duplicate_end_node->id;
				new_scope_node->next_node = duplicate_end_node;
			}

			AbstractNode* duplicate_start_node = duplicate_local_scope
				->nodes[this->successful_location_starts[s_index]->id];
			switch (duplicate_start_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)duplicate_start_node;

					action_node->next_node_id = new_scope_node->id;
					action_node->next_node = new_scope_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)duplicate_start_node;

					scope_node->next_node_id = new_scope_node->id;
					scope_node->next_node = new_scope_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)duplicate_start_node;

					if (this->successful_location_is_branch[s_index]) {
						branch_node->branch_next_node_id = new_scope_node->id;
						branch_node->branch_next_node = new_scope_node;
					} else {
						branch_node->original_next_node_id = new_scope_node->id;
						branch_node->original_next_node = new_scope_node;
					}
				}
				break;
			case NODE_TYPE_RETURN:
				{
					ReturnNode* return_node = (ReturnNode*)duplicate_start_node;

					if (this->successful_location_is_branch[s_index]) {
						return_node->passed_next_node_id = new_scope_node->id;
						return_node->passed_next_node = new_scope_node;
					} else {
						return_node->skipped_next_node_id = new_scope_node->id;
						return_node->skipped_next_node = new_scope_node;
					}
				}
				break;
			case NODE_TYPE_MARKOV:
				{
					MarkovNode* markov_node = (MarkovNode*)duplicate_start_node;

					markov_node->next_node_id = new_scope_node->id;
					markov_node->next_node = new_scope_node;
				}
				break;
			}
		}
		this->successful_scope_nodes.clear();

		this->new_scope = NULL;
	}
}
