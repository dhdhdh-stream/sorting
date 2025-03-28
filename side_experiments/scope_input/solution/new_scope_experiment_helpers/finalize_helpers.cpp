#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void NewScopeExperiment::finalize(Solution* duplicate) {
	Scope* parent_scope = (Scope*)this->scope_context;
	parent_scope->new_scope_experiment = NULL;

	for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
		this->test_location_starts[t_index]->experiment = NULL;
	}
	for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
		this->successful_location_starts[s_index]->experiment = NULL;
	}

	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		cout << "NewScopeExperiment success" << endl;

		Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];

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
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)it->second;

					for (int f_index = 0; f_index < (int)branch_node->factors.size(); f_index++) {
						if (branch_node->factors[f_index].type == INPUT_TYPE_OBS) {
							ObsNode* obs_node = (ObsNode*)this->new_scope->nodes[branch_node->factors[f_index].node_context.back()];
							Factor* factor = obs_node->factors[branch_node->factors[f_index].factor_index];

							factor->link(duplicate);

							obs_node->is_used = true;
						}
					}

					#if defined(MDEBUG) && MDEBUG
					branch_node->verify_key = this;
					#endif /* MDEBUG */
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)it->second;

					for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
						Factor* factor = obs_node->factors[f_index];

						for (int i_index = 0; i_index < (int)factor->inputs.size(); i_index++) {
							for (int l_index = 1; l_index < (int)factor->inputs[i_index].scope_context.size(); l_index++) {
								factor->inputs[i_index].scope_context[l_index] =
									duplicate->scopes[factor->inputs[i_index].scope_context[l_index]->id];
							}
						}

						for (int i_index = 0; i_index < (int)factor->inputs.size(); i_index++) {
							if (factor->inputs[i_index].type == INPUT_TYPE_OBS) {
								Scope* scope = factor->inputs[i_index].scope_context.back();
								AbstractNode* node = scope->nodes[factor->inputs[i_index].node_context.back()];
								switch (node->type) {
								case NODE_TYPE_BRANCH:
									{
										BranchNode* branch_node = (BranchNode*)node;
										branch_node->is_used = true;
									}
									break;
								case NODE_TYPE_OBS:
									{
										ObsNode* obs_node = (ObsNode*)node;

										if (factor->inputs[i_index].factor_index != -1) {
											Factor* i_factor = obs_node->factors[factor->inputs[i_index].factor_index];

											i_factor->link(duplicate);
										}

										obs_node->is_used = true;
									}
									break;
								}
							}
						}
					}
				}
				break;
			}
		}

		this->new_scope->child_scopes = duplicate_local_scope->child_scopes;
		map<Scope*, vector<Input>> new_child_scope_inputs;
		for (map<Scope*, vector<Input>>::iterator it = this->new_scope->child_scope_inputs.begin();
				it != this->new_scope->child_scope_inputs.end(); it++) {
			for (int i_index = 0; i_index < (int)it->second.size(); i_index++) {
				for (int l_index = 1; l_index < (int)it->second[i_index].scope_context.size(); l_index++) {
					it->second[i_index].scope_context[l_index] =
						duplicate->scopes[it->second[i_index].scope_context[l_index]->id];
				}
			}

			new_child_scope_inputs[duplicate->scopes[it->first->id]] = it->second;
		}
		this->new_scope->child_scope_inputs = new_child_scope_inputs;

		duplicate_local_scope->child_scopes.push_back(this->new_scope);
		for (int i_index = 0; i_index < (int)this->new_scope_inputs.size(); i_index++) {
			for (int l_index = 0; l_index < (int)this->new_scope_inputs[i_index].scope_context.size(); l_index++) {
				this->new_scope_inputs[i_index].scope_context[l_index] =
					duplicate->scopes[this->new_scope_inputs[i_index].scope_context[l_index]->id];
			}
		}
		duplicate_local_scope->child_scope_inputs[this->new_scope] = this->new_scope_inputs;

		#if defined(MDEBUG) && MDEBUG
		duplicate->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		duplicate->verify_seeds = this->verify_seeds;
		duplicate->verify_can_random = this->verify_can_random;
		#endif /* MDEBUG */

		clean_scope(this->new_scope,
					duplicate);

		ObsNode* new_local_ending_node = NULL;

		for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
			ScopeNode* new_scope_node = this->successful_scope_nodes[s_index];
			new_scope_node->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[new_scope_node->id] = new_scope_node;

			if (new_scope_node->next_node == NULL) {
				if (new_local_ending_node == NULL) {
					new_local_ending_node = new ObsNode();
					new_local_ending_node->parent = duplicate_local_scope;
					new_local_ending_node->id = duplicate_local_scope->node_counter;
					duplicate_local_scope->node_counter++;

					for (map<int, AbstractNode*>::iterator it = duplicate_local_scope->nodes.begin();
							it != duplicate_local_scope->nodes.end(); it++) {
						if (it->second->type == NODE_TYPE_OBS) {
							ObsNode* obs_node = (ObsNode*)it->second;
							if (obs_node->next_node == NULL) {
								obs_node->next_node_id = new_local_ending_node->id;
								obs_node->next_node = new_local_ending_node;

								new_local_ending_node->ancestor_ids.push_back(obs_node->id);

								break;
							}
						}
					}

					duplicate_local_scope->nodes[new_local_ending_node->id] = new_local_ending_node;

					new_local_ending_node->next_node_id = -1;
					new_local_ending_node->next_node = NULL;
				}

				new_scope_node->next_node_id = new_local_ending_node->id;
				new_scope_node->next_node = new_local_ending_node;

				new_local_ending_node->ancestor_ids.push_back(new_scope_node->id);
			} else {
				AbstractNode* duplicate_end_node = duplicate_local_scope->nodes[new_scope_node->next_node->id];

				new_scope_node->next_node_id = duplicate_end_node->id;
				new_scope_node->next_node = duplicate_end_node;

				duplicate_end_node->ancestor_ids.push_back(new_scope_node->id);
			}

			AbstractNode* duplicate_start_node = duplicate_local_scope
				->nodes[this->successful_location_starts[s_index]->id];
			switch (duplicate_start_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)duplicate_start_node;

					for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
						if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
							action_node->next_node->ancestor_ids.erase(
								action_node->next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					action_node->next_node_id = new_scope_node->id;
					action_node->next_node = new_scope_node;

					new_scope_node->ancestor_ids.push_back(action_node->id);
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)duplicate_start_node;

					for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
						if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
							scope_node->next_node->ancestor_ids.erase(
								scope_node->next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					scope_node->next_node_id = new_scope_node->id;
					scope_node->next_node = new_scope_node;

					new_scope_node->ancestor_ids.push_back(scope_node->id);
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)duplicate_start_node;

					if (this->successful_location_is_branch[s_index]) {
						for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
							if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
								branch_node->branch_next_node->ancestor_ids.erase(
									branch_node->branch_next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}

						branch_node->branch_next_node_id = new_scope_node->id;
						branch_node->branch_next_node = new_scope_node;

						new_scope_node->ancestor_ids.push_back(branch_node->id);
					} else {
						for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
							if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
								branch_node->original_next_node->ancestor_ids.erase(
									branch_node->original_next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}

						branch_node->original_next_node_id = new_scope_node->id;
						branch_node->original_next_node = new_scope_node;

						new_scope_node->ancestor_ids.push_back(branch_node->id);
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)duplicate_start_node;

					for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
						if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
							obs_node->next_node->ancestor_ids.erase(
								obs_node->next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					obs_node->next_node_id = new_scope_node->id;
					obs_node->next_node = new_scope_node;

					new_scope_node->ancestor_ids.push_back(obs_node->id);
				}
				break;
			}
		}
		this->successful_scope_nodes.clear();

		this->new_scope = NULL;
	}
}
