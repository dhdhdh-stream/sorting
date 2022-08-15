#ifndef SOLUTION_NODE_LOOP_END_H
#define SOLUTION_NODE_LOOP_END_H

class SolutionNodeLoopEnd : public SolutionNode {
public:
	SolutionNode* halt;
	SolutionNode* no_halt;

	std::vector<int> halt_network_inputs_state_indexes;
	Network* halt_network;
	std::string halt_network_name;

	int start_index;
};

#endif /* SOLUTION_NODE_LOOP_END_H */