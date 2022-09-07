#ifndef START_SCOPE_H
#define START_SCOPE_H

const int START_SCOPE_STATE_ENTER = 0;
const int START_SCOPE_STATE_EXIT = 1;

class StartScope : public SolutionNode {
public:
	std::vector<SolutionNode*> path;

	// score_network ran at start
};

#endif /* START_SCOPE_H */