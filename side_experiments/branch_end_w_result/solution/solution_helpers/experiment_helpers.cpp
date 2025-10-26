#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

bool is_new_scope_iter(SolutionWrapper* wrapper) {
	if (wrapper->solution->timestamp % 8 == 4) {
		return true;
	} else {
		return false;
	}
}

/**
 * - don't prioritize exploring new nodes as new scopes change explore
 */
void gather_helper(ScopeHistory* scope_history,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_START:
		case NODE_TYPE_ACTION:
		case NODE_TYPE_OBS:
			if (node->experiment == NULL) {
				uniform_int_distribution<int> select_distribution(0, node_count);
				node_count++;
				if (select_distribution(generator) == 0) {
					explore_node = node;
					explore_is_branch = false;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node;
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

				gather_helper(scope_node_history->scope_history,
							  node_count,
							  explore_node,
							  explore_is_branch);

				if (node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = scope_node;
						explore_is_branch = false;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			if (node->experiment == NULL) {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				if (branch_node_history->is_branch) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = true;
					}
				} else {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
					}
				}
			}
			break;
		}
	}
}

void create_branch_experiment(ScopeHistory* scope_history,
							  SolutionWrapper* wrapper) {
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	gather_helper(scope_history,
				  node_count,
				  explore_node,
				  explore_is_branch);

	// if (explore_node != NULL) {
	if (explore_node != NULL
			&& explore_node->type == NODE_TYPE_OBS) {
		if (wrapper->solution->last_new_scope != NULL) {
			if (explore_node->parent == wrapper->solution->last_new_scope) {
				BranchExperiment* new_experiment = new BranchExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch,
					wrapper);
				wrapper->curr_branch_experiment = new_experiment;
			}
		} else {
			BranchExperiment* new_experiment = new BranchExperiment(
				explore_node->parent,
				explore_node,
				explore_is_branch,
				wrapper);
			wrapper->curr_branch_experiment = new_experiment;
		}
	}
}

void create_new_scope_overall_experiment(ScopeHistory* scope_history,
										 SolutionWrapper* wrapper) {
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	gather_helper(scope_history,
				  node_count,
				  explore_node,
				  explore_is_branch);

	if (explore_node != NULL) {
		if (explore_node->parent->nodes.size() >= NEW_SCOPE_MIN_NODES) {
			Scope* new_scope = create_new_scope(explore_node->parent,
												wrapper,
												false);
			if (new_scope != NULL) {
				wrapper->curr_new_scope_experiment = new NewScopeOverallExperiment(
					new_scope,
					explore_node->parent);
			}
		}
	}
}

void create_new_scope_experiment_gather_helper(
		ScopeHistory* scope_history,
		int& node_count,
		AbstractNode*& explore_node,
		bool& explore_is_branch,
		Scope* scope_context) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_START:
		case NODE_TYPE_ACTION:
		case NODE_TYPE_OBS:
			if (scope_history->scope == scope_context
					&& node->experiment == NULL) {
				uniform_int_distribution<int> select_distribution(0, node_count);
				node_count++;
				if (select_distribution(generator) == 0) {
					explore_node = node;
					explore_is_branch = false;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node;
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

				create_new_scope_experiment_gather_helper(
					scope_node_history->scope_history,
					node_count,
					explore_node,
					explore_is_branch,
					scope_context);

				if (scope_history->scope == scope_context
						&& node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = scope_node;
						explore_is_branch = false;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			if (scope_history->scope == scope_context
					&& node->experiment == NULL) {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				if (branch_node_history->is_branch) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = true;
					}
				} else {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
					}
				}
			}
			break;
		}
	}
}

void create_new_scope_experiment(ScopeHistory* scope_history,
								 SolutionWrapper* wrapper) {
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	create_new_scope_experiment_gather_helper(
		scope_history,
		node_count,
		explore_node,
		explore_is_branch,
		wrapper->curr_new_scope_experiment->scope_context);

	if (explore_node != NULL) {
		NewScopeExperiment* new_experiment = new NewScopeExperiment(
			explore_node->parent,
			explore_node,
			explore_is_branch,
			wrapper->curr_new_scope_experiment->new_scope);
		wrapper->curr_new_scope_experiment->curr_experiment = new_experiment;
	}
}

bool still_instances_possible_helper(ScopeHistory* scope_history,
									 Scope* scope_context) {
	Scope* scope = scope_history->scope;

	if (scope == scope_context) {
		for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
				h_it != scope_history->node_histories.end(); h_it++) {
			AbstractNode* node = h_it->second->node;
			if (node->experiment == NULL) {
				return true;
			}
		}
	} else {
		if (scope->child_scopes.find(scope_context) != scope->child_scopes.end()) {
			for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
					h_it != scope_history->node_histories.end(); h_it++) {
				AbstractNode* node = h_it->second->node;
				if (node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
					bool inner_result = still_instances_possible_helper(
						scope_node_history->scope_history,
						scope_context);
					if (inner_result) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool still_instances_possible(NewScopeOverallExperiment* experiment) {
	uniform_int_distribution<int> distribution(0, experiment->new_scope_histories.size()-1);
	for (int t_index = 0; t_index < 10; t_index++) {
		int index = distribution(generator);
		bool curr_result = still_instances_possible_helper(
			experiment->new_scope_histories[index],
			experiment->scope_context);
		if (curr_result) {
			return true;
		}
	}

	return false;
}
