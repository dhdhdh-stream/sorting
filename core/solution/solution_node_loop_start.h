#ifndef SOLUTION_NODE_LOOP_START_H
#define SOLUTION_NODE_LOOP_START_H

class SolutionNodeLoopStart : public SolutionNode {
public:
	std::vector<int> states_on;

	SolutionNode* next;

	SolutionNode* previous;
	SolutionNode* loop_in;

	Network* score_network;
};

#endif /* SOLUTION_NODE_LOOP_START_H */