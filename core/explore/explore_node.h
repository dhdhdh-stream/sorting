#ifndef EXPLORE_NODE_H
#define EXPLORE_NODE_H

const int EXPLORE_ROOT = 0;
const int EXPLORE_JUMP = 1;
const int EXPLORE_IF_START_JUMP = 2;
const int EXPLORE_LOOP = 3;
const int EXPLORE_STATE = 4;
const int EXPLORE_GOTO = 5;

class ExploreNode {
public:
	int type;

	// create immediately after evaluation
	std::vector<ExploreNode*> children;

	int count;
	double average_score;
	double average_misguess;

	virtual void process(Solution* solution) = 0;
};

#endif /* EXPLORE_NODE_H */