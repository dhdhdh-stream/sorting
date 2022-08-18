#ifndef EXPLORE_H
#define EXPLORE_H

#include <vector>

#include "explore_node.h"
#include "solution.h"

class Explore {
public:
	ExploreNode* root;

	Solution* solution;

	ExploreNode* current_node;
	int iter_index;

	// 5x 5->1 path candidates, 5x 5->1 state candidates?

	Explore(SolutionNode* parent);
	~Explore();

	void setup_cycle();
	void iteration();
};

#endif /* EXPLORE_H */