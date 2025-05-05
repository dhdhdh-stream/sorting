#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void clean_scope(Scope* scope) {
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

				solution->clean_inputs(scope,
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
					if (!next_obs_node->is_used) {
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

						solution->clean_inputs(scope,
											   next_obs_node->id);

						scope->nodes.erase(next_obs_node->id);
						delete next_obs_node;

						removed_node = true;
						break;
					} else if (curr_obs_node->id != 0
							&& !curr_obs_node->is_used) {
						for (int a_index = 0; a_index < (int)next_obs_node->ancestor_ids.size(); a_index++) {
							if (next_obs_node->ancestor_ids[a_index] == curr_obs_node->id) {
								next_obs_node->ancestor_ids.erase(
									next_obs_node->ancestor_ids.begin() + a_index);
								break;
							}
						}

						for (int a_index = 0; a_index < (int)curr_obs_node->ancestor_ids.size(); a_index++) {
							AbstractNode* node = scope->nodes[curr_obs_node->ancestor_ids[a_index]];
							switch (node->type) {
							case NODE_TYPE_ACTION:
								{
									ActionNode* action_node = (ActionNode*)node;

									action_node->next_node_id = next_obs_node->id;
									action_node->next_node = next_obs_node;
								}
								break;
							case NODE_TYPE_SCOPE:
								{
									ScopeNode* scope_node = (ScopeNode*)node;

									scope_node->next_node_id = next_obs_node->id;
									scope_node->next_node = next_obs_node;
								}
								break;
							case NODE_TYPE_BRANCH:
								{
									BranchNode* branch_node = (BranchNode*)node;

									if (branch_node->original_next_node == curr_obs_node) {
										branch_node->original_next_node_id = next_obs_node->id;
										branch_node->original_next_node = next_obs_node;
									}
									if (branch_node->branch_next_node == curr_obs_node) {
										branch_node->branch_next_node_id = next_obs_node->id;
										branch_node->branch_next_node = next_obs_node;
									}
								}
								break;
							case NODE_TYPE_OBS:
								{
									ObsNode* obs_node = (ObsNode*)node;

									obs_node->next_node_id = next_obs_node->id;
									obs_node->next_node = next_obs_node;
								}
								break;
							}

							next_obs_node->ancestor_ids.push_back(node->id);
						}

						solution->clean_inputs(scope,
											   curr_obs_node->id);

						scope->nodes.erase(curr_obs_node->id);
						delete curr_obs_node;

						removed_node = true;
						break;
					}
				}
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
}
