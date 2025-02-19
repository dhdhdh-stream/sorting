#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "obs_node.h"
#include "scope_node.h"

using namespace std;

void Scope::measure_activate(Problem* problem,
							 RunHelper& run_helper,
							 ScopeHistory* history) {
	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		// if (curr_node == NULL
		// 		|| run_helper.early_exit) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;
				node->measure_activate(curr_node,
									   problem,
									   run_helper,
									   history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)curr_node;
				node->measure_activate(curr_node,
									   problem,
									   run_helper,
									   history);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;
				node->measure_activate(curr_node,
									   problem,
									   run_helper,
									   history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)curr_node;
				node->measure_activate(curr_node,
									   problem,
									   run_helper,
									   history);
			}
			break;
		}
	}
}
