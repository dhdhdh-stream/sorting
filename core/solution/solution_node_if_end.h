#ifndef SOLUTION_NODE_IF_END_H
#define SOLUTION_NODE_IF_END_H

class SolutionNodeIfEnd : public SolutionNode {
public:
	SolutionNode* next;

	int start_index;
	
	Network* score_network;
};

#endif /* SOLUTION_NODE_IF_END_H */