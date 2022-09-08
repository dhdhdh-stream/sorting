#ifndef JUMP_SCOPE_H
#define JUMP_SCOPE_H

const int JUMP_SCOPE_STATE_ENTER = 0;
const int JUMP_SCOPE_STATE_IF = 1;
const int JUMP_SCOPE_STATE_EXIT = 2;

const int JUMP_SCOPE_IF_EXPLORE_TYPE_APPEND = 0;
const int JUMP_SCOPE_IF_EXPLORE_TYPE_BRANCH_START = 1;

class JumpScope : public SolutionNode {
public:
	int num_states; // TODO: try 0

	std::vector<SolutionNode*> top_path;

	// double if_explore_weight;
	// int if_explore_type;
	// int if_explore_state;
	// std::vector<SolutionNode*> if_explore_potential_nodes;
	// Network* if_explore_score_network;	// train greedily
	// Network* if_explore_full_score_network;
	// std::vector<FlatNetwork*> if_explore_state_networks;

	std::vector<std::vector<SolutionNode*>> children_nodes;
	std::vector<Network*> children_score_networks;

};

#endif /* JUMP_SCOPE_H */