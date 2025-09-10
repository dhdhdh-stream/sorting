#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void gather_helper(ScopeHistory* scope_history,
				   set<pair<AbstractNode*, bool>>& possible_starts) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		if (node->experiment == NULL) {
			switch (node->type) {
			case NODE_TYPE_START:
			case NODE_TYPE_ACTION:
			case NODE_TYPE_OBS:
				possible_starts.insert({node, false});
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					gather_helper(scope_node_history->scope_history,
								  possible_starts);
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					possible_starts.insert({node, branch_node_history->is_branch});
				}
				break;
			}
		}
	}
}

void create_experiment(SolutionWrapper* wrapper) {
	set<pair<AbstractNode*, bool>> possible_starts;
	gather_helper(wrapper->scope_histories[0],
				  possible_starts);

	if (possible_starts.size() > 0) {
		uniform_int_distribution<int> possible_distribution(0, possible_starts.size()-1);
		pair<AbstractNode*, bool> start = *next(possible_starts.begin(), possible_distribution(generator));
		AbstractNode* node_context = start.first;
		bool is_branch = start.second;

		Scope* scope_context = node_context->parent;

		vector<AbstractNode*> possible_exits;

		AbstractNode* starting_node;
		switch (node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)node_context;
				starting_node = start_node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node_context;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node_context;
				starting_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node_context;
				if (is_branch) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node_context;
				starting_node = obs_node->next_node;
			}
			break;
		}

		scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		int random_index;
		geometric_distribution<int> exit_distribution(0.2);
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		AbstractNode* exit_next_node = possible_exits[random_index];

		ExploreExperiment* new_experiment = new ExploreExperiment(
			scope_context,
			node_context,
			is_branch,
			exit_next_node);
		node_context->experiment = new_experiment;
	}
}
