#if defined(MDEBUG) && MDEBUG

#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_verify_activate_helper(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 ScopeHistory* history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			ScopeNodeHistory* node_history = new ScopeNodeHistory();
			node_history->index = (int)history->node_histories.size();
			history->node_histories[node] = node_history;
			node->verify_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  node_history);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->verify_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  history->node_histories);
		}

		break;
	case NODE_TYPE_INFO_SCOPE:
		{
			InfoScopeNode* node = (InfoScopeNode*)curr_node;
			node->verify_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  history->node_histories);
		}

		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* node = (InfoBranchNode*)curr_node;
			node->verify_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  history->node_histories);
		}

		break;
	}
}

void Scope::verify_activate(Problem* problem,
							vector<ContextLayer>& context,
							RunHelper& run_helper,
							ScopeHistory* history) {
	if (context.size() > solution->scopes.size() + 1) {
		run_helper.exceeded_limit = true;
		return;
	}

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		run_helper.num_actions++;
		if (run_helper.num_actions > solution->num_actions_limit) {
			run_helper.exceeded_limit = true;
			break;
		}

		node_verify_activate_helper(curr_node,
									problem,
									context,
									run_helper,
									history);
	}

	if (this->verify_key != NULL) {
		cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
		cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

		if ((int)history->node_histories.size() != this->verify_scope_history_sizes[0]) {
			cout << "history->node_histories.size(): " << history->node_histories.size() << endl;
			cout << "this->verify_scope_history_sizes[0]: " << this->verify_scope_history_sizes[0] << endl;

			cout << "run_helper.num_actions: " << run_helper.num_actions << endl;
			cout << "solution->num_actions_limit: " << solution->num_actions_limit << endl;

			throw invalid_argument("new scope verify fail");
		}
		this->verify_scope_history_sizes.erase(this->verify_scope_history_sizes.begin());
	}
}

#endif /* MDEBUG */