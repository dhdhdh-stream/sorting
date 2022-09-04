#ifndef JUMP_SCOPE_H
#define JUMP_SCOPE_H

const int JUMP_SCOPE_EXPLORE_TYPE_TOP = 0;
const int JUMP_SCOPE_EXPLORE_TYPE_NEW_BRANCH = 1;
const int JUMP_SCOPE_EXPLORE_TYPE_EXISTING_BRANCH = 2;
const int JUMP_SCOPE_EXPLORE_TYPE_OUT = 3;

// to know status of jump, push back scope 0 initially, switch to 1 after jump

class JumpScope : public SolutionNode {
public:
	int num_states;

	std::vector<SolutionNode*> top_path;

	double if_explore_weight;
	double end_explore_weight;

	std::vector<std::vector<SolutionNode*>> children_nodes;
	std::vector<Network*> children_score_networks;
	// used to evaluate replacement
	// update at end of jump_scope using prediction from last node
	// so will bias towards new branches probably, but that's OK?
	std::vector<double> children_misguess;

	int explore_type;

	SolutionNode* top_explore_node;

	std::vector<Action> explore_branch_flat;
	std::vector<SolutionNodeAction*> explore_branch_fold;
	std::vector<AbstractAction*> branch_explore_nodes;

	std::vector<double> explore_new_branch_children_diffs_positive;
	std::vector<double> explore_new_branch_children_diffs_negative;
};

#endif /* JUMP_SCOPE_H */