#ifndef EXPLORE_NODE_H
#define EXPLORE_NODE_H

const int EXPLORE_ROOT = 0;
const int EXPLORE_JUMP = 1;
const int EXPLORE_IF_START_JUMP = 2;
const int EXPLORE_LOOP_END_HALT_JUMP = 3;
const int EXPLORE_LOOP = 4;
const int EXPLORE_STATE = 5;
// for gotos, instead be good about jumping to end of scope

class ExploreNode {
public:
	Explore* explore;

	int type;

	std::vector<ExploreNode*> children;

	// TODO: Consider impact from exploration

	int count;
	double average_score;
	double average_misguess;

	virtual void process() = 0;
};

#endif /* EXPLORE_NODE_H */