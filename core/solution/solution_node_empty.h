#ifndef SOLUTION_NODE_EMPTY_H
#define SOLUTION_NODE_EMPTY_H

// only appear by themselves, and can only set state
class SolutionNodeEmpty : public SolutionNode {
public:
	std::vector<int> state_network_local_index;
	std::vector<Network*> state_networks;
};

#endif /* SOLUTION_NODE_EMPTY_H */