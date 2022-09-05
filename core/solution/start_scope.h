#ifndef START_SCOPE_H
#define START_SCOPE_H

class StartScope : public SolutionNode {
public:
	std::vector<SolutionNode*> path;

	// score_network ran at start
};

#endif /* START_SCOPE_H */