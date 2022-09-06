#ifndef JUMP_SCOPE_H
#define JUMP_SCOPE_H

const int JUMP_SCOPE_IF_EXPLORE_TYPE_APPEND = 0;
const int JUMP_SCOPE_IF_EXPLORE_TYPE_BRANCH_START = 1;

class JumpScope : public SolutionNode {
public:
	int num_states; // TODO: try 0

	std::vector<SolutionNode*> top_path;

	double if_explore_weight;
	int if_explore_type;
	int if_explore_state;
	std::vector<SolutionNode*> if_explore_potential_nodes;
	Network* if_explore_score_network;	// train greedily
	Network* if_explore_full_score_network;
	std::vector<FlatNetwork*> if_explore_state_networks;

	double end_explore_weight;
	int end_explore_state;


	std::vector<std::vector<SolutionNode*>> children_nodes;
	std::vector<Network*> children_score_networks;
	// used to evaluate replacement
	// update at end of jump_scope using prediction from last node
	// so will bias towards new branches probably, but that's OK?
	std::vector<double> children_misguess;

	std::vector<Action> explore_branch_flat;
	std::vector<SolutionNodeAction*> explore_branch_fold;
	std::vector<AbstractAction*> branch_explore_nodes;

	std::vector<double> explore_new_branch_children_diffs_positive;
	std::vector<double> explore_new_branch_children_diffs_negative;
};

#endif /* JUMP_SCOPE_H */