#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void NewScopeExperiment::finalize(Solution* duplicate) {
	Scope* parent_scope = (Scope*)this->scope_context;
	parent_scope->new_scope_experiment = NULL;

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
		cout << "NewScopeExperiment success" << endl;

		this->new_scope->id = duplicate->scopes.size();
		duplicate->scopes.push_back(this->new_scope);

		for (map<int, AbstractNode*>::iterator it = this->new_scope->nodes.begin();
				it != this->new_scope->nodes.end(); it++) {
			switch (it->second->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)it->second;

					for (int i_index = 0; i_index < (int)action_node->input_scope_contexts.size(); i_index++) {
						for (int l_index = 1; l_index < (int)action_node->input_scope_contexts[i_index].size(); l_index++) {
							action_node->input_scope_contexts[i_index][l_index] =
								duplicate->scopes[action_node->input_scope_contexts[i_index][l_index]->id];
						}
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)it->second;
					scope_node->scope = duplicate->scopes[scope_node->scope->id];
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)it->second;

					for (int i_index = 0; i_index < (int)branch_node->inputs.size(); i_index++) {
						for (int l_index = 1; l_index < (int)branch_node->inputs[i_index].first.first.size(); l_index++) {
							branch_node->inputs[i_index].first.first[l_index] =
								duplicate->scopes[branch_node->inputs[i_index].first.first[l_index]->id];
						}
					}

					for (int i_index = 0; i_index < (int)branch_node->inputs.size(); i_index++) {
						Scope* scope = branch_node->inputs[i_index].first.first.back();
						AbstractNode* node = scope->nodes[branch_node->inputs[i_index].first.second.back()];
						switch (node->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNode* input_action_node = (ActionNode*)node;

								bool is_existing = false;
								for (int ii_index = 0; ii_index < (int)input_action_node->input_scope_contexts.size(); ii_index++) {
									if (input_action_node->input_scope_contexts[ii_index] == branch_node->inputs[i_index].first.first
											&& input_action_node->input_node_context_ids[ii_index] == branch_node->inputs[i_index].first.second
											&& input_action_node->input_obs_indexes[ii_index] == branch_node->inputs[i_index].second) {
										is_existing = true;
										break;
									}
								}
								if (!is_existing) {
									input_action_node->input_scope_contexts.push_back(branch_node->inputs[i_index].first.first);
									input_action_node->input_node_context_ids.push_back(branch_node->inputs[i_index].first.second);
									input_action_node->input_obs_indexes.push_back(branch_node->inputs[i_index].second);
								}
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNode* input_branch_node = (BranchNode*)node;

								bool is_existing = false;
								for (int ii_index = 0; ii_index < (int)input_branch_node->input_scope_contexts.size(); ii_index++) {
									if (input_branch_node->input_scope_contexts[ii_index] == branch_node->inputs[i_index].first.first
											&& input_branch_node->input_node_context_ids[ii_index] == branch_node->inputs[i_index].first.second) {
										is_existing = true;
										break;
									}
								}
								if (!is_existing) {
									input_branch_node->input_scope_contexts.push_back(branch_node->inputs[i_index].first.first);
									input_branch_node->input_node_context_ids.push_back(branch_node->inputs[i_index].first.second);
								}
							}
							break;
						}
					}

					for (int i_index = 0; i_index < (int)branch_node->input_scope_contexts.size(); i_index++) {
						for (int l_index = 1; l_index < (int)branch_node->input_scope_contexts[i_index].size(); l_index++) {
							branch_node->input_scope_contexts[i_index][l_index] =
								duplicate->scopes[branch_node->input_scope_contexts[i_index][l_index]->id];
						}
					}

					#if defined(MDEBUG) && MDEBUG
					branch_node->verify_key = this;
					#endif /* MDEBUG */
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

		this->new_scope->child_scopes = duplicate_local_scope->child_scopes;
		duplicate_local_scope->child_scopes.push_back(this->new_scope);
		this->new_scope->existing_scopes = duplicate_local_scope->existing_scopes;

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
			}
		}
		this->successful_scope_nodes.clear();

		this->new_scope = NULL;
	}
}
