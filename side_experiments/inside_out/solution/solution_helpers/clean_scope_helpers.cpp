#include "solution_helpers.h"

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

const int SCOPE_EXCEEDED_NUM_NODES = 100;

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
	 * - remove duplicate ObsNodes
	 * 
	 * - clean for all scopes as ObsNodes could have been added during experiments
	 */
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		while (true) {
			bool removed_node = false;

			for (map<int, AbstractNode*>::iterator it = wrapper->solution->scopes[s_index]->nodes.begin();
					it != wrapper->solution->scopes[s_index]->nodes.end(); it++) {
				if (it->second->type == NODE_TYPE_OBS) {
					ObsNode* curr_obs_node = (ObsNode*)it->second;
					if (curr_obs_node->next_node != NULL
							&& curr_obs_node->next_node->type == NODE_TYPE_OBS
							&& curr_obs_node->next_node->ancestor_ids.size() == 1) {
						ObsNode* next_obs_node = (ObsNode*)curr_obs_node->next_node;

						for (int f_index = 0; f_index < (int)next_obs_node->factors.size(); f_index++) {
							Factor* new_factor = new Factor(next_obs_node->factors[f_index],
															wrapper->solution);
							curr_obs_node->factors.push_back(new_factor);

							wrapper->solution->replace_factor(wrapper->solution->scopes[s_index],
															  next_obs_node->id,
															  f_index,
															  curr_obs_node->id,
															  curr_obs_node->factors.size()-1);
						}

						wrapper->solution->replace_obs_node(wrapper->solution->scopes[s_index],
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

						wrapper->solution->clean_inputs(wrapper->solution->scopes[s_index],
														next_obs_node->id);

						wrapper->solution->scopes[s_index]->nodes.erase(next_obs_node->id);
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

					for (int f_index = 0; f_index < (int)original_obs_node->factors.size(); f_index++) {
						Factor* new_factor = new Factor(original_obs_node->factors[f_index],
														wrapper->solution);
						merge_obs_node->factors.push_back(new_factor);

						wrapper->solution->replace_factor(scope,
														  original_obs_node->id,
														  f_index,
														  merge_obs_node->id,
														  merge_obs_node->factors.size()-1);
					}

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

					for (int f_index = 0; f_index < (int)branch_obs_node->factors.size(); f_index++) {
						Factor* new_factor = new Factor(branch_obs_node->factors[f_index],
														wrapper->solution);
						merge_obs_node->factors.push_back(new_factor);

						wrapper->solution->replace_factor(scope,
														  branch_obs_node->id,
														  f_index,
														  merge_obs_node->id,
														  merge_obs_node->factors.size()-1);
					}

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

	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		it->second->is_init = true;
	}
}

void check_generalize(Scope* scope_to_generalize,
					  SolutionWrapper* wrapper) {
	if (scope_to_generalize->nodes.size() > SCOPE_EXCEEDED_NUM_NODES
			&& !scope_to_generalize->generalized) {
		cout << "generalize " << scope_to_generalize->id << endl;

		Scope* new_scope = new Scope();
		new_scope->node_counter = 0;

		new_scope->child_scopes = scope_to_generalize->child_scopes;
		new_scope->child_scopes.push_back(scope_to_generalize);

		ObsNode* start_node = new ObsNode();
		start_node->parent = new_scope;
		start_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[start_node->id] = start_node;

		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->parent = new_scope;
		new_scope_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[new_scope_node->id] = new_scope_node;

		new_scope_node->scope = scope_to_generalize;

		ObsNode* end_node = new ObsNode();
		end_node->parent = new_scope;
		end_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[end_node->id] = end_node;

		start_node->next_node_id = new_scope_node->id;
		start_node->next_node = new_scope_node;

		new_scope_node->next_node_id = end_node->id;
		new_scope_node->next_node = end_node;

		end_node->next_node_id = -1;
		end_node->next_node = NULL;

		wrapper->solution->replace_scope(scope_to_generalize,
										 new_scope,
										 new_scope_node->id);

		if (scope_to_generalize->id == 0) {
			wrapper->solution->scopes.insert(wrapper->solution->scopes.begin(), new_scope);
		} else {
			wrapper->solution->scopes.push_back(new_scope);
		}

		scope_to_generalize->generalized = true;

		for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
			wrapper->solution->scopes[s_index]->id = s_index;
		}
	}
}
