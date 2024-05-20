#include "new_action_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "eval.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void NewActionExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		cout << "NewActionExperiment success" << endl;

		Scope* new_scope = new Scope();
		new_scope->id = duplicate->scopes.size();
		duplicate->scopes.push_back(new_scope);

		new_scope->node_counter = 0;

		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = new_scope;
		starting_noop_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		starting_noop_node->action = Action(ACTION_NOOP);
		new_scope->nodes[starting_noop_node->id] = starting_noop_node;

		map<AbstractNode*, AbstractNode*> node_mappings;

		switch (this->starting_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* original_action_node = (ActionNode*)this->starting_node;

				ActionNode* new_action_node = new ActionNode();
				new_action_node->parent = new_scope;
				new_action_node->id = new_scope->node_counter;
				new_scope->node_counter++;
				new_scope->nodes[new_action_node->id] = new_action_node;

				new_action_node->action = original_action_node->action;

				node_mappings[original_action_node] = new_action_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* original_scope_node = (ScopeNode*)this->starting_node;

				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->parent = new_scope;
				new_scope_node->id = new_scope->node_counter;
				new_scope->node_counter++;
				new_scope->nodes[new_scope_node->id] = new_scope_node;

				new_scope_node->scope = duplicate->scopes[original_scope_node->scope->id];

				node_mappings[original_scope_node] = new_scope_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* original_branch_node = (BranchNode*)this->starting_node;

				BranchNode* new_branch_node = new BranchNode();
				new_branch_node->parent = new_scope;
				new_branch_node->id = new_scope->node_counter;
				new_scope->node_counter++;
				new_scope->nodes[new_branch_node->id] = new_branch_node;

				/**
				 * - add inputs after
				 */

				node_mappings[original_branch_node] = new_branch_node;
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* original_info_branch_node = (InfoBranchNode*)this->starting_node;

				InfoBranchNode* new_info_branch_node = new InfoBranchNode();
				new_info_branch_node->parent = new_scope;
				new_info_branch_node->id = new_scope->node_counter;
				new_scope->node_counter++;
				new_scope->nodes[new_info_branch_node->id] = new_info_branch_node;

				new_info_branch_node->scope = duplicate->info_scopes[original_info_branch_node->scope->id];
				new_info_branch_node->is_negate = original_info_branch_node->is_negate;

				node_mappings[original_info_branch_node] = new_info_branch_node;
			}
			break;
		}
		for (set<AbstractNode*>::iterator node_it = this->included_nodes.begin();
				node_it != this->included_nodes.end(); node_it++) {
			switch ((*node_it)->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* original_action_node = (ActionNode*)(*node_it);

					ActionNode* new_action_node = new ActionNode();
					new_action_node->parent = new_scope;
					new_action_node->id = new_scope->node_counter;
					new_scope->node_counter++;
					new_scope->nodes[new_action_node->id] = new_action_node;

					new_action_node->action = original_action_node->action;

					node_mappings[original_action_node] = new_action_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* original_scope_node = (ScopeNode*)(*node_it);

					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->parent = new_scope;
					new_scope_node->id = new_scope->node_counter;
					new_scope->node_counter++;
					new_scope->nodes[new_scope_node->id] = new_scope_node;

					new_scope_node->scope = duplicate->scopes[original_scope_node->scope->id];

					node_mappings[original_scope_node] = new_scope_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* original_branch_node = (BranchNode*)(*node_it);

					BranchNode* new_branch_node = new BranchNode();
					new_branch_node->parent = new_scope;
					new_branch_node->id = new_scope->node_counter;
					new_scope->node_counter++;
					new_scope->nodes[new_branch_node->id] = new_branch_node;

					/**
					 * - add inputs after
					 */

					node_mappings[original_branch_node] = new_branch_node;
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNode* original_info_branch_node = (InfoBranchNode*)(*node_it);

					InfoBranchNode* new_info_branch_node = new InfoBranchNode();
					new_info_branch_node->parent = new_scope;
					new_info_branch_node->id = new_scope->node_counter;
					new_scope->node_counter++;
					new_scope->nodes[new_info_branch_node->id] = new_info_branch_node;

					new_info_branch_node->scope = duplicate->info_scopes[original_info_branch_node->scope->id];
					new_info_branch_node->is_negate = original_info_branch_node->is_negate;

					node_mappings[original_info_branch_node] = new_info_branch_node;
				}
				break;
			}
		}

		starting_noop_node->next_node_id = node_mappings[this->starting_node]->id;
		starting_noop_node->next_node = node_mappings[this->starting_node];

		ActionNode* new_ending_node = new ActionNode();
		new_ending_node->parent = new_scope;
		new_ending_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_ending_node->action = Action(ACTION_NOOP);
		new_scope->nodes[new_ending_node->id] = new_ending_node;
		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		switch (this->starting_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* original_action_node = (ActionNode*)this->starting_node;
				ActionNode* new_action_node = (ActionNode*)node_mappings[original_action_node];

				map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
					.find(original_action_node->next_node);
				if (it == node_mappings.end()) {
					new_action_node->next_node_id = new_ending_node->id;
					new_action_node->next_node = new_ending_node;
				} else {
					new_action_node->next_node_id = it->second->id;
					new_action_node->next_node = it->second;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* original_scope_node = (ScopeNode*)this->starting_node;
				ScopeNode* new_scope_node = (ScopeNode*)node_mappings[original_scope_node];

				map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
					.find(original_scope_node->next_node);
				if (it == node_mappings.end()) {
					new_scope_node->next_node_id = new_ending_node->id;
					new_scope_node->next_node = new_ending_node;
				} else {
					new_scope_node->next_node_id = it->second->id;
					new_scope_node->next_node = it->second;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* original_branch_node = (BranchNode*)this->starting_node;
				BranchNode* new_branch_node = (BranchNode*)node_mappings[original_branch_node];

				new_branch_node->original_average_score = original_branch_node->original_average_score;
				new_branch_node->branch_average_score = original_branch_node->branch_average_score;

				/**
				 * - inputs not needed for starting_node
				 */

				new_branch_node->original_network = NULL;
				new_branch_node->branch_network = NULL;

				map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings
					.find(original_branch_node->original_next_node);
				if (original_it == node_mappings.end()) {
					new_branch_node->original_next_node_id = new_ending_node->id;
					new_branch_node->original_next_node = new_ending_node;
				} else {
					new_branch_node->original_next_node_id = original_it->second->id;
					new_branch_node->original_next_node = original_it->second;
				}
				map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings
					.find(original_branch_node->branch_next_node);
				if (branch_it == node_mappings.end()) {
					new_branch_node->branch_next_node_id = new_ending_node->id;
					new_branch_node->branch_next_node = new_ending_node;
				} else {
					new_branch_node->branch_next_node_id = branch_it->second->id;
					new_branch_node->branch_next_node = branch_it->second;
				}
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* original_info_branch_node = (InfoBranchNode*)this->starting_node;
				InfoBranchNode* new_info_branch_node = (InfoBranchNode*)node_mappings[original_info_branch_node];

				map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings
					.find(original_info_branch_node->original_next_node);
				if (original_it == node_mappings.end()) {
					new_info_branch_node->original_next_node_id = new_ending_node->id;
					new_info_branch_node->original_next_node = new_ending_node;
				} else {
					new_info_branch_node->original_next_node_id = original_it->second->id;
					new_info_branch_node->original_next_node = original_it->second;
				}
				map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings
					.find(original_info_branch_node->branch_next_node);
				if (branch_it == node_mappings.end()) {
					new_info_branch_node->branch_next_node_id = new_ending_node->id;
					new_info_branch_node->branch_next_node = new_ending_node;
				} else {
					new_info_branch_node->branch_next_node_id = branch_it->second->id;
					new_info_branch_node->branch_next_node = branch_it->second;
				}
			}
			break;
		}
		for (set<AbstractNode*>::iterator node_it = this->included_nodes.begin();
				node_it != this->included_nodes.end(); node_it++) {
			switch ((*node_it)->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* original_action_node = (ActionNode*)(*node_it);
					ActionNode* new_action_node = (ActionNode*)node_mappings[original_action_node];

					map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
						.find(original_action_node->next_node);
					if (it == node_mappings.end()) {
						new_action_node->next_node_id = new_ending_node->id;
						new_action_node->next_node = new_ending_node;
					} else {
						new_action_node->next_node_id = it->second->id;
						new_action_node->next_node = it->second;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* original_scope_node = (ScopeNode*)(*node_it);
					ScopeNode* new_scope_node = (ScopeNode*)node_mappings[original_scope_node];

					map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
						.find(original_scope_node->next_node);
					if (it == node_mappings.end()) {
						new_scope_node->next_node_id = new_ending_node->id;
						new_scope_node->next_node = new_ending_node;
					} else {
						new_scope_node->next_node_id = it->second->id;
						new_scope_node->next_node = it->second;
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* original_branch_node = (BranchNode*)(*node_it);
					BranchNode* new_branch_node = (BranchNode*)node_mappings[original_branch_node];

					new_branch_node->original_average_score = original_branch_node->original_average_score;
					new_branch_node->branch_average_score = original_branch_node->branch_average_score;

					for (int i_index = 0; i_index < (int)original_branch_node->input_scope_contexts.size(); i_index++) {
						if (original_branch_node->input_scope_contexts[i_index].size() == 0) {
							new_branch_node->input_scope_context_ids.push_back(vector<int>());
							new_branch_node->input_scope_contexts.push_back(vector<Scope*>());
							new_branch_node->input_node_context_ids.push_back(vector<int>());
							new_branch_node->input_node_contexts.push_back(vector<AbstractNode*>());
							new_branch_node->input_obs_indexes.push_back(-1);
						} else {
							map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
								.find(original_branch_node->input_node_contexts[i_index][0]);
							if (it == node_mappings.end()) {
								new_branch_node->input_scope_context_ids.push_back(vector<int>());
								new_branch_node->input_scope_contexts.push_back(vector<Scope*>());
								new_branch_node->input_node_context_ids.push_back(vector<int>());
								new_branch_node->input_node_contexts.push_back(vector<AbstractNode*>());
								new_branch_node->input_obs_indexes.push_back(-1);
							} else {
								new_branch_node->input_scope_context_ids.push_back(original_branch_node->input_scope_context_ids[i_index]);
								new_branch_node->input_scope_context_ids[i_index][0] = new_scope->id;

								new_branch_node->input_node_context_ids.push_back(original_branch_node->input_node_context_ids[i_index]);
								new_branch_node->input_node_context_ids[i_index][0] = it->second->id;

								vector<Scope*> scope_context;
								vector<AbstractNode*> node_context;
								for (int l_index = 0; l_index < (int)new_branch_node->input_scope_context_ids[i_index].size(); l_index++) {
									Scope* duplicate_scope = duplicate->scopes[new_branch_node->input_scope_context_ids[i_index][l_index]];
									AbstractNode* duplicate_node = duplicate_scope->nodes[new_branch_node->input_node_context_ids[i_index][l_index]];

									scope_context.push_back(duplicate_scope);
									node_context.push_back(duplicate_node);
								}
								new_branch_node->input_scope_contexts.push_back(scope_context);
								new_branch_node->input_node_contexts.push_back(node_context);

								new_branch_node->input_obs_indexes.push_back(original_branch_node->input_obs_indexes[i_index]);
							}
						}
					}

					new_branch_node->linear_original_input_indexes = original_branch_node->linear_original_input_indexes;
					new_branch_node->linear_original_weights = original_branch_node->linear_original_weights;
					new_branch_node->linear_branch_input_indexes = original_branch_node->linear_branch_input_indexes;
					new_branch_node->linear_branch_weights = original_branch_node->linear_branch_weights;

					new_branch_node->original_network_input_indexes = original_branch_node->original_network_input_indexes;
					if (original_branch_node->original_network == NULL) {
						new_branch_node->original_network = NULL;
					} else {
						new_branch_node->original_network = new Network(original_branch_node->original_network);
					}
					new_branch_node->branch_network_input_indexes = original_branch_node->branch_network_input_indexes;
					if (original_branch_node->branch_network == NULL) {
						new_branch_node->branch_network = NULL;
					} else {
						new_branch_node->branch_network = new Network(original_branch_node->branch_network);
					}

					map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings
						.find(original_branch_node->original_next_node);
					if (original_it == node_mappings.end()) {
						new_branch_node->original_next_node_id = new_ending_node->id;
						new_branch_node->original_next_node = new_ending_node;
					} else {
						new_branch_node->original_next_node_id = original_it->second->id;
						new_branch_node->original_next_node = original_it->second;
					}
					map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings
						.find(original_branch_node->branch_next_node);
					if (branch_it == node_mappings.end()) {
						new_branch_node->branch_next_node_id = new_ending_node->id;
						new_branch_node->branch_next_node = new_ending_node;
					} else {
						new_branch_node->branch_next_node_id = branch_it->second->id;
						new_branch_node->branch_next_node = branch_it->second;
					}
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNode* original_info_branch_node = (InfoBranchNode*)(*node_it);
					InfoBranchNode* new_info_branch_node = (InfoBranchNode*)node_mappings[original_info_branch_node];

					map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings
						.find(original_info_branch_node->original_next_node);
					if (original_it == node_mappings.end()) {
						new_info_branch_node->original_next_node_id = new_ending_node->id;
						new_info_branch_node->original_next_node = new_ending_node;
					} else {
						new_info_branch_node->original_next_node_id = original_it->second->id;
						new_info_branch_node->original_next_node = original_it->second;
					}
					map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings
						.find(original_info_branch_node->branch_next_node);
					if (branch_it == node_mappings.end()) {
						new_info_branch_node->branch_next_node_id = new_ending_node->id;
						new_info_branch_node->branch_next_node = new_ending_node;
					} else {
						new_info_branch_node->branch_next_node_id = branch_it->second->id;
						new_info_branch_node->branch_next_node = branch_it->second;
					}
				}
				break;
			}
		}

		new_scope->eval = new Eval(new_scope);
		new_scope->eval->init();

		#if defined(MDEBUG) && MDEBUG
		duplicate->verify_key = this;
		duplicate->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		duplicate->verify_seeds = this->verify_seeds;

		new_scope->verify_key = this;
		new_scope->verify_scope_history_sizes = this->verify_scope_history_sizes;
		#endif /* MDEBUG */

		ActionNode* new_local_ending_node = NULL;

		Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];
		for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = duplicate_local_scope;
			new_scope_node->id = duplicate_local_scope->node_counter;
			duplicate_local_scope->node_counter++;
			duplicate_local_scope->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->scope = new_scope;

			if (this->successful_location_exits[s_index] == NULL) {
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
				AbstractNode* duplicate_end_node = duplicate_local_scope->nodes[this->successful_location_exits[s_index]->id];
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
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNode* info_branch_node = (InfoBranchNode*)duplicate_start_node;

					if (this->successful_location_is_branch[s_index]) {
						info_branch_node->branch_next_node_id = new_scope_node->id;
						info_branch_node->branch_next_node = new_scope_node;
					} else {
						info_branch_node->original_next_node_id = new_scope_node->id;
						info_branch_node->original_next_node = new_scope_node;
					}
				}
				break;
			}
		}
	}

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
}
