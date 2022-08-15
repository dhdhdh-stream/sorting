#ifndef SOLUTION_NODE_GOTO_START_H
#define SOLUTION_NODE_GOTO_START_H

class SolutionNodeGotoStart : public SolutionNode {
public:
	std::vector<int> states_on;

	SolutionNode* next;

	Network* score_network;
};

#endif /* SOLUTION_NODE_GOTO_START_H */