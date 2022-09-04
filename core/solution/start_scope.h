#ifndef START_SCOPE_H
#define START_SCOPE_H

class StartScope : public SolutionNode {
public:
	SolutionNode* start;

	// score_network ran at start

	SolutionNode* explore_node;
};

#endif /* START_SCOPE_H */