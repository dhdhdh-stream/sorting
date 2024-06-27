#include "branch_tree_node.h"

using namespace std;

void BranchTreeNode::activate(Problem* problem,
							  map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	bool is_branch;
	switch (this->branch_node->type) {
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->branch_node;

			BranchNodeHistory* history = new BranchNodeHistory();
			history->index = (int)node_histories.size();
			node_histories[branch_node] = history;

			vector<double> input_vals(branch_node->input_scope_contexts.size(), 0.0);
			for (int i_index = 0; i_index < (int)branch_node->input_scope_contexts.size(); i_index++) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = node_histories.find(
					branch_node->input_node_contexts[i_index].back());
				if (it != node_histories.end()) {
					switch (it->first->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							input_vals[i_index] = action_node_history->obs_snapshot[branch_node->input_obs_indexes[i_index]];
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							input_vals[i_index] = branch_node_history->score;
						}
						break;
					case NODE_TYPE_INFO_BRANCH:
						{
							InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
							if (info_branch_node_history->is_branch) {
								input_vals[i_index] = 1.0;
							} else {
								input_vals[i_index] = -1.0;
							}
						}
						break;
					}
				}
			}
			branch_node->network->activate(input_vals);
			history->score = branch_node->network->output->acti_vals[0];

			if (history->score >= 0.0) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}
		break;
	}

	if (is_branch) {
		this->branch_path->activate(problem,
									node_histories);
	} else {
		this->original_path->activate(problem,
									  node_histories);
	}
}
