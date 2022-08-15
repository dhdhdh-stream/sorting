#ifndef SOLUTION_NODE_NORMAL_H
#define SOLUTION_NODE_NORMAL_H

class SolutionNodeNormal : public SolutionNode {
public:
	Action action;

	std::vector<std::vector<int>> state_network_inputs_state_indexes;
	std::vector<Network*> state_networks;
	std::vector<int> state_networks_target_states;
	std::vector<std::string> state_network_names;

	SolutionNode* next;

	SolutionNode* previous;

	Network* score_network;
};

#endif /* SOLUTION_NODE_NORMAL_H */