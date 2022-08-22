#ifndef EXPLORE_H
#define EXPLORE_H

#include <mutex>
#include <vector>

#include "explore_node.h"
#include "solution.h"

class Explore {
public:
	ExploreNode* root;

	Solution* solution;

	ExploreNode* current_node;

	std::mutex mtx;

	Explore();
	~Explore();

	void setup_cycle();
};

#endif /* EXPLORE_H */