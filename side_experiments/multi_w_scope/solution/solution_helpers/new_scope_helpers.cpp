#include "solution_helpers.h"

#include <map>
#include <utility>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "minesweeper.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

bool new_scope_in_place_helper(Problem* problem,
							   AbstractNode* start_node,
							   map<AbstractNode*, pair<int,int>>& mapping) {
	AbstractNode* curr_node = start_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		Minesweeper* minesweeper = (Minesweeper*)problem;
		pair<int,int> curr_loc = {
			minesweeper->current_x,
			minesweeper->current_y
		};

		map<AbstractNode*, pair<int,int>>::iterator it = mapping.find(curr_node);
		if (it == mapping.end()) {
			mapping[curr_node] = curr_loc;
		} else {
			if (it->second != curr_loc) {
				return false;
			}
		}

		switch (curr_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)curr_node;

				problem->perform_action(action_node->action);

				curr_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)curr_node;
				curr_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)curr_node;

				Problem* copy_problem = problem->copy_snapshot();
				bool result = new_scope_in_place_helper(copy_problem,
														branch_node->branch_next_node,
														mapping);
				delete copy_problem;

				if (!result) {
					return false;
				}

				curr_node = branch_node->original_next_node;
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)curr_node;
				curr_node = obs_node->next_node;
			}
			break;
		}
	}

	return true;
}

bool new_scope_in_place(Scope* scope) {
	map<AbstractNode*, pair<int,int>> mapping;

	Problem* problem = problem_type->get_problem();
	bool result = new_scope_in_place_helper(problem,
											scope->nodes[0],
											mapping);
	delete problem;

	if (!result) {
		return false;
	}

	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_OBS) {
			ObsNode* obs_node = (ObsNode*)it->second;
			if (obs_node->next_node == NULL) {
				pair<int,int> loc = mapping[obs_node];
				if (loc.first != 4 || loc.second != 4) {
					return false;
				}
				break;
			}
		}
	}

	return true;
}
