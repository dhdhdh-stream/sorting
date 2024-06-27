#include "transformations_helpers.h"

using namespace std;

BranchTreeNode* solution_to_tree_helper(AbstractNode* start_node) {
	BranchTreeNode* branch_tree_node = new BranchTreeNode();
	branch_tree_node->branch_node = start_node;

	BranchEndNode* branch_end_node;
	switch (start_node->type) {
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)start_node;
			branch_end_node = branch_node->branch_end_node;
		}
		break;
	}

	{
		SequenceTreeNode* original_path = new SequenceTreeNode();
		AbstractNode* curr_node;
		switch (start_node->type) {
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)start_node;
				curr_node = branch_node->original_next_node;
			}
			break;
		}

		while (curr_node != branch_end_node) {
			switch (curr_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)curr_node;

					ActionTreeNode* action_tree_node = new ActionTreeNode();
					action_tree_node->action_node = action_node;
					original_path->nodes.push_back(action_tree_node);

					curr_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)curr_node;

					BranchTreeNode* branch_tree_node = solution_to_tree_helper(curr_node);
					original_path->nodes.push_back(branch_tree_node);

					curr_node = branch_node->branch_end_node->next_node;
				}
				break;
			}
		}
		branch_tree_node->original_path = original_path;
	}

	{
		SequenceTreeNode* branch_path = new SequenceTreeNode();
		AbstractNode* curr_node;
		switch (start_node->type) {
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)start_node;
				curr_node = branch_node->original_next_node;
			}
			break;
		}

		while (curr_node != branch_end_node) {
			switch (curr_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)curr_node;

					ActionTreeNode* action_tree_node = new ActionTreeNode();
					action_tree_node->action_node = action_node;
					branch_path->nodes.push_back(action_tree_node);

					curr_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)curr_node;

					BranchTreeNode* branch_tree_node = solution_to_tree_helper(curr_node);
					branch_path->nodes.push_back(branch_tree_node);

					curr_node = branch_node->branch_end_node->next_node;
				}
				break;
			}
		}
		branch_tree_node->branch_path = branch_path;
	}

	return branch_tree_node;
}

SequenceTreeNode* solution_to_tree(Scope* parent_scope) {
	SequenceTreeNode* sequence_tree_node = new SequenceTreeNode();

	AbstractNode* curr_node = parent_scope->nodes[0];

	while (true) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)curr_node;

				ActionTreeNode* action_tree_node = new ActionTreeNode();
				action_tree_node->action_node = action_node;
				sequence_tree_node->nodes.push_back(action_tree_node);

				curr_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)curr_node;

				BranchTreeNode* branch_tree_node = solution_to_tree_helper(curr_node);
				sequence_tree_node->nodes.push_back(branch_tree_node);

				curr_node = branch_node->branch_end_node->next_node;
			}
			break;
		}
	}

	return sequence_tree_node;
}
