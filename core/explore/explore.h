#ifndef EXPLORE_H
#define EXPLORE_H

#include <mutex>
#include <vector>

#include "explore_node.h"
#include "solution.h"

class ExploreNode;
class Solution;
class Explore {
public:
	ExploreNode* root;

	Solution* solution;

	ExploreNode* current_node;

	std::mutex mtx;

	Explore();
	Explore(std::ifstream& save_file);
	~Explore();

	void setup_cycle();

	void save();
};

#endif /* EXPLORE_H */