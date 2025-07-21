#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper) {
	/**
	 * - remove no longer accessible nodes
	 */
	while (true) {
		bool removed_node = false;

		set<int> next_node_ids;
		next_node_ids.insert(0);
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			switch (it->second->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)it->second;
					next_node_ids.insert(action_node->next_node_id);
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)it->second;
					next_node_ids.insert(scope_node->next_node_id);
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)it->second;
					next_node_ids.insert(branch_node->original_next_node_id);
					next_node_ids.insert(branch_node->branch_next_node_id);
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)it->second;
					next_node_ids.insert(obs_node->next_node_id);
				}
				break;
			}
		}

		map<int, AbstractNode*>::iterator it = scope->nodes.begin();
		while (it != scope->nodes.end()) {
			set<int>::iterator needed_it = next_node_ids.find(it->first);
			if (needed_it == next_node_ids.end()) {
				removed_node = true;

				wrapper->solution->clean_inputs(scope,
												it->first);

				switch (it->second->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)it->second;

						for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
							if (action_node->next_node->ancestor_ids[a_index] == it->second->id) {
								action_node->next_node->ancestor_ids.erase(
									action_node->next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)it->second;

						for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
							if (scope_node->next_node->ancestor_ids[a_index] == it->second->id) {
								scope_node->next_node->ancestor_ids.erase(
									scope_node->next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)it->second;

						for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
							if (branch_node->original_next_node->ancestor_ids[a_index] == it->second->id) {
								branch_node->original_next_node->ancestor_ids.erase(
									branch_node->original_next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}

						for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
							if (branch_node->branch_next_node->ancestor_ids[a_index] == it->second->id) {
								branch_node->branch_next_node->ancestor_ids.erase(
									branch_node->branch_next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)it->second;

						for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
							if (obs_node->next_node->ancestor_ids[a_index] == it->second->id) {
								obs_node->next_node->ancestor_ids.erase(
									obs_node->next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
					}
					break;
				}

				delete it->second;
				it = scope->nodes.erase(it);
			} else {
				it++;
			}
		}

		if (!removed_node) {
			break;
		}
	}

	/**
	 * - add needed ObsNodes
	 */
	vector<pair<AbstractNode*,bool>> obs_node_needed;
	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;

				if (action_node->next_node->type != NODE_TYPE_OBS
						|| action_node->next_node->ancestor_ids.size() > 1) {
					obs_node_needed.push_back({action_node, false});
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)it->second;

				if (scope_node->next_node->type != NODE_TYPE_OBS
						|| scope_node->next_node->ancestor_ids.size() > 1) {
					obs_node_needed.push_back({scope_node, false});
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;

				if (branch_node->original_next_node->type != NODE_TYPE_OBS
						|| branch_node->original_next_node->ancestor_ids.size() > 1) {
					obs_node_needed.push_back({branch_node, false});
				}
				if (branch_node->branch_next_node->type != NODE_TYPE_OBS
						|| branch_node->branch_next_node->ancestor_ids.size() > 1) {
					obs_node_needed.push_back({branch_node, true});
				}
			}
			break;
		}
	}
	for (int n_index = 0; n_index < (int)obs_node_needed.size(); n_index++) {
		ObsNode* new_obs_node = new ObsNode();
		new_obs_node->parent = scope;
		new_obs_node->id = scope->node_counter;
		scope->node_counter++;
		scope->nodes[new_obs_node->id] = new_obs_node;

		switch (obs_node_needed[n_index].first->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)obs_node_needed[n_index].first;

				for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
					if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
						action_node->next_node->ancestor_ids.erase(
							action_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				action_node->next_node->ancestor_ids.push_back(new_obs_node->id);

				new_obs_node->next_node_id = action_node->next_node_id;
				new_obs_node->next_node = action_node->next_node;

				action_node->next_node_id = new_obs_node->id;
				action_node->next_node = new_obs_node;

				new_obs_node->ancestor_ids.push_back(action_node->id);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)obs_node_needed[n_index].first;

				for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
					if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
						scope_node->next_node->ancestor_ids.erase(
							scope_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				scope_node->next_node->ancestor_ids.push_back(new_obs_node->id);

				new_obs_node->next_node_id = scope_node->next_node_id;
				new_obs_node->next_node = scope_node->next_node;

				scope_node->next_node_id = new_obs_node->id;
				scope_node->next_node = new_obs_node;

				new_obs_node->ancestor_ids.push_back(scope_node->id);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)obs_node_needed[n_index].first;

				if (obs_node_needed[n_index].second) {
					for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->branch_next_node->ancestor_ids.erase(
								branch_node->branch_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					branch_node->branch_next_node->ancestor_ids.push_back(new_obs_node->id);

					new_obs_node->next_node_id = branch_node->branch_next_node_id;
					new_obs_node->next_node = branch_node->branch_next_node;

					branch_node->branch_next_node_id = new_obs_node->id;
					branch_node->branch_next_node = new_obs_node;
				} else {
					for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->original_next_node->ancestor_ids.erase(
								branch_node->original_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					branch_node->original_next_node->ancestor_ids.push_back(new_obs_node->id);

					new_obs_node->next_node_id = branch_node->original_next_node_id;
					new_obs_node->next_node = branch_node->original_next_node;

					branch_node->original_next_node_id = new_obs_node->id;
					branch_node->original_next_node = new_obs_node;
				}

				new_obs_node->ancestor_ids.push_back(branch_node->id);
			}
			break;
		}
	}
	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		if (it->second->ancestor_ids.size() > 1 && it->second->type != NODE_TYPE_OBS) {
			ObsNode* new_obs_node = new ObsNode();
			new_obs_node->parent = scope;
			new_obs_node->id = scope->node_counter;
			scope->node_counter++;
			scope->nodes[new_obs_node->id] = new_obs_node;

			new_obs_node->next_node_id = it->first;
			new_obs_node->next_node = it->second;

			for (int a_index = 0; a_index < (int)it->second->ancestor_ids.size(); a_index++) {
				ObsNode* obs_node = (ObsNode*)scope->nodes[it->second->ancestor_ids[a_index]];
				obs_node->next_node_id = new_obs_node->id;
				obs_node->next_node = new_obs_node;
			}

			new_obs_node->ancestor_ids = it->second->ancestor_ids;
			it->second->ancestor_ids = {new_obs_node->id};
		}
	}

	/**
	 * - remove useless BranchNodes
	 */
	while (true) {
		bool removed_node = false;

		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_BRANCH) {
				BranchNode* branch_node = (BranchNode*)it->second;
				ObsNode* original_obs_node = (ObsNode*)branch_node->original_next_node;
				ObsNode* branch_obs_node = (ObsNode*)branch_node->branch_next_node;
				if (original_obs_node->next_node == branch_obs_node->next_node) {
					ObsNode* merge_obs_node = (ObsNode*)original_obs_node->next_node;

					wrapper->solution->replace_obs_node(scope,
														original_obs_node->id,
														merge_obs_node->id);

					for (int a_index = 0; a_index < (int)merge_obs_node->ancestor_ids.size(); a_index++) {
						if (merge_obs_node->ancestor_ids[a_index] == original_obs_node->id) {
							merge_obs_node->ancestor_ids.erase(merge_obs_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					wrapper->solution->clean_inputs(scope,
													original_obs_node->id);

					wrapper->solution->replace_obs_node(scope,
														branch_obs_node->id,
														merge_obs_node->id);

					for (int a_index = 0; a_index < (int)merge_obs_node->ancestor_ids.size(); a_index++) {
						if (merge_obs_node->ancestor_ids[a_index] == branch_obs_node->id) {
							merge_obs_node->ancestor_ids.erase(merge_obs_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					wrapper->solution->clean_inputs(scope,
													branch_obs_node->id);

					wrapper->solution->clean_inputs(scope,
													branch_node->id);

					ObsNode* previous_obs_node = (ObsNode*)scope->nodes[branch_node->ancestor_ids[0]];
					previous_obs_node->next_node_id = merge_obs_node->id;
					previous_obs_node->next_node = merge_obs_node;
					merge_obs_node->ancestor_ids.push_back(previous_obs_node->id);

					scope->nodes.erase(original_obs_node->id);
					delete original_obs_node;
					scope->nodes.erase(branch_obs_node->id);
					delete branch_obs_node;
					scope->nodes.erase(branch_node->id);
					delete branch_node;

					removed_node = true;
					break;
				}
			}
		}

		if (!removed_node) {
			break;
		}
	}

	/**
	 * - remove duplicate ObsNodes
	 */
	while (true) {
		bool removed_node = false;

		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* curr_obs_node = (ObsNode*)it->second;
				if (curr_obs_node->next_node != NULL
						&& curr_obs_node->next_node->type == NODE_TYPE_OBS
						&& curr_obs_node->next_node->ancestor_ids.size() == 1) {
					ObsNode* next_obs_node = (ObsNode*)curr_obs_node->next_node;

					wrapper->solution->replace_obs_node(scope,
														next_obs_node->id,
														curr_obs_node->id);

					if (next_obs_node->next_node != NULL) {
						for (int a_index = 0; a_index < (int)next_obs_node->next_node->ancestor_ids.size(); a_index++) {
							if (next_obs_node->next_node->ancestor_ids[a_index] == next_obs_node->id) {
								next_obs_node->next_node->ancestor_ids.erase(
									next_obs_node->next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
						next_obs_node->next_node->ancestor_ids.push_back(curr_obs_node->id);
					}
					curr_obs_node->next_node_id = next_obs_node->next_node_id;
					curr_obs_node->next_node = next_obs_node->next_node;

					wrapper->solution->clean_inputs(scope,
													next_obs_node->id);

					scope->nodes.erase(next_obs_node->id);
					delete next_obs_node;

					removed_node = true;
					break;
				}
			}
		}

		if (!removed_node) {
			break;
		}
	}

	/**
	 * - eval Factor is_meaningful
	 */
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		for (int f_index = 0; f_index < (int)wrapper->solution->scopes[s_index]->factors.size(); f_index++) {
			Factor* factor = wrapper->solution->scopes[s_index]->factors[f_index];

			bool has_meaningful = false;
			for (int i_index = 0; i_index < (int)factor->inputs.size(); i_index++) {
				if (factor->inputs[i_index].factor_index == -1) {
					has_meaningful = true;
					break;
				} else {
					Scope* input_scope = factor->inputs[i_index].scope_context.back();
					if (input_scope->factors[factor->inputs[i_index].factor_index]->is_meaningful) {
						has_meaningful = true;
						break;
					}
				}
			}

			factor->is_meaningful = has_meaningful;
		}
	}

	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		it->second->is_init = true;
	}

	/**
	 * - clear explore as nodes removed
	 */
	for (int h_index = 0; h_index < (int)wrapper->solution->explore_scope_histories.size(); h_index++) {
		delete wrapper->solution->explore_scope_histories[h_index];
	}
	wrapper->solution->explore_scope_histories.clear();
	wrapper->solution->explore_target_val_histories.clear();
}
