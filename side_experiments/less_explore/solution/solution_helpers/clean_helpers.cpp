#include "solution_helpers.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

/**
 * - to_replace removed from replacement's ancestor_ids outside
 */
void replace_next(AbstractNode* node,
				  BranchNode* to_replace,
				  AbstractNode* replacement) {
	switch (node->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)node;
			noop_node->next_node_id = replacement->id;
			noop_node->next_node = replacement;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node;
			action_node->next_node_id = replacement->id;
			action_node->next_node = replacement;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)node;
			scope_node->next_node_id = replacement->id;
			scope_node->next_node = replacement;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node;
			/**
			 * - safe to be called twice for useless BranchNodes
			 */
			if (branch_node->original_next_node == to_replace) {
				branch_node->original_next_node_id = replacement->id;
				branch_node->original_next_node = replacement;
			} else if (branch_node->branch_next_node == to_replace) {
				branch_node->branch_next_node_id = replacement->id;
				branch_node->branch_next_node = replacement;
			}
		}
		break;
	}

	replacement->ancestor_ids.push_back(node->id);
}

void clean_scope(Scope* scope) {
	set<AbstractNode*> experiment_endpoints;
	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)it->second;
				for (int e_index = 0; e_index < (int)noop_node->experiments.size(); e_index++) {
					if (noop_node->experiments[e_index] != NULL
							&& noop_node->experiments[e_index]->exit_next_node != NULL) {
						experiment_endpoints.insert(noop_node->experiments[e_index]->exit_next_node);
					}
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;
				for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
					if (action_node->experiments[e_index] != NULL
							&& action_node->experiments[e_index]->exit_next_node != NULL) {
						experiment_endpoints.insert(action_node->experiments[e_index]->exit_next_node);
					}
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)it->second;
				for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
					if (scope_node->experiments[e_index] != NULL
							&& scope_node->experiments[e_index]->exit_next_node != NULL) {
						experiment_endpoints.insert(scope_node->experiments[e_index]->exit_next_node);
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				for (int e_index = 0; e_index < (int)branch_node->original_experiments.size(); e_index++) {
					if (branch_node->original_experiments[e_index] != NULL
							&& branch_node->original_experiments[e_index]->exit_next_node != NULL) {
						experiment_endpoints.insert(branch_node->original_experiments[e_index]->exit_next_node);
					}
				}
				for (int e_index = 0; e_index < (int)branch_node->branch_experiments.size(); e_index++) {
					if (branch_node->branch_experiments[e_index] != NULL
							&& branch_node->branch_experiments[e_index]->exit_next_node != NULL) {
						experiment_endpoints.insert(branch_node->branch_experiments[e_index]->exit_next_node);
					}
				}
			}
			break;
		}
	}

	/**
	 * - remove useless BranchNodes
	 */
	#if defined(MDEBUG) && MDEBUG
	#else
	while (true) {
		bool removed_node = false;

		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_BRANCH
					&& experiment_endpoints.find(it->second) == experiment_endpoints.end()) {
				BranchNode* branch_node = (BranchNode*)it->second;

				bool is_remove = false;
				AbstractNode* replacement;
				if (branch_node->ramp == 0) {
					is_remove = true;
					replacement = branch_node->original_next_node;
				} else if (branch_node->consec_original >= CONSEC_DEPRECATE_LIMIT) {
					is_remove = true;
					replacement = branch_node->original_next_node;
				} else if (branch_node->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
					is_remove = true;
					replacement = branch_node->branch_next_node;
				} else if (branch_node->original_next_node == branch_node->branch_next_node) {
					is_remove = true;
					replacement = branch_node->original_next_node;
				}

				if (is_remove) {
					for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->original_next_node->ancestor_ids.erase(branch_node->original_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->branch_next_node->ancestor_ids.erase(branch_node->branch_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					for (int a_index = 0; a_index < (int)branch_node->ancestor_ids.size(); a_index++) {
						AbstractNode* node = scope->nodes[branch_node->ancestor_ids[a_index]];
						replace_next(node,
									 branch_node,
									 replacement);
					}

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
	#endif /* MDEBUG */

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
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)it->second;
					if (noop_node->next_node != NULL) {
						next_node_ids.insert(noop_node->next_node_id);
					}
				}
				break;
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
			}
		}

		map<int, AbstractNode*>::iterator it = scope->nodes.begin();
		while (it != scope->nodes.end()) {
			if (next_node_ids.find(it->first) == next_node_ids.end()
					&& experiment_endpoints.find(it->second) == experiment_endpoints.end()) {
				removed_node = true;

				switch (it->second->type) {
				case NODE_TYPE_NOOP:
					{
						NoopNode* noop_node = (NoopNode*)it->second;

						for (int a_index = 0; a_index < (int)noop_node->next_node->ancestor_ids.size(); a_index++) {
							if (noop_node->next_node->ancestor_ids[a_index] == it->second->id) {
								noop_node->next_node->ancestor_ids.erase(
									noop_node->next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
					}
					break;
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
}
