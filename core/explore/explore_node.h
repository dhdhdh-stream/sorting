#ifndef EXPLORE_NODE_H
#define EXPLORE_NODE_H

const int EXPLORE_ROOT = 0;
const int EXPLORE_NEW_JUMP = 1;
const int EXPLORE_APPEND_JUMP = 2;
const int EXPLORE_LOOP = 3;
const int EXPLORE_STATE = 4;

class ExploreNode {
public:
	Explore* explore;

	int type;

	std::vector<ExploreNode*> children;

	// TODO: Consider impact from exploration

	int count;
	double average_score;
	double average_misguess;

	virtual ~ExploreNode() {
		for (int c_index = 0; c_index < (int)this->children.size(); c_index++) {
			delete this->children[c_index];
		}
	};

	virtual void process() = 0;
};

#endif /* EXPLORE_NODE_H */