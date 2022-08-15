#ifndef SOLUTION_NODE_IF_START_H
#define SOLUTION_NODE_IF_START_H

class SolutionNodeIfStart : public SolutionNode {
public:
	std::vector<SolutionNode*> children_nodes;
	std::vector<int> children_score_networks_inputs_state_indexes;
	std::vector<Network*> children_score_networks;
	std::vector<std::string> children_score_network_names;

	std::vector<bool> children_on;

	int end_index;

	std::vector<int> states_on;

	SolutionNode* previous;
};

#endif /* SOLUTION_NODE_IF