#if defined(MDEBUG) && MDEBUG

#include "scope.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void node_verify_activate_helper(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* node = (ActionNode*)curr_node;
		node->verify_activate(curr_node,
							  problem,
							  context,
							  exit_depth,
							  exit_node,
							  run_helper);
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* node = (ScopeNode*)curr_node;
		node->verify_activate(curr_node,
							  problem,
							  context,
							  exit_depth,
							  exit_node,
							  run_helper);
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;

		bool is_branch;
		node->verify_activate(problem,
							  is_branch,
							  context,
							  run_helper);

		if (is_branch) {
			curr_node = node->branch_next_node;
		} else {
			curr_node = node->original_next_node;
		}
	} else {
		ExitNode* node = (ExitNode*)curr_node;

		if (node->is_exit) {
			run_helper.has_exited = true;
		} else {
			if (node->exit_depth == 0) {
				curr_node = node->exit_node;
			} else {
				exit_depth = node->exit_depth-1;
				exit_node = node->exit_node;
			}
		}
	}
}

void Scope::verify_activate(Problem* problem,
							vector<ContextLayer>& context,
							int& exit_depth,
							AbstractNode*& exit_node,
							RunHelper& run_helper) {
	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	run_helper.curr_depth++;

	AbstractNode* curr_node = this->starting_node;
	while (true) {
		if (run_helper.has_exited
				|| run_helper.exceeded_limit
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		node_verify_activate_helper(curr_node,
									problem,
									context,
									exit_depth,
									exit_node,
									run_helper);
	}

	run_helper.curr_depth--;
}

#endif /* MDEBUG */