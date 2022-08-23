#ifndef EXPLORE_NODE_ROOT_H
#define EXPLORE_NODE_ROOT_H

#include "explore_node.h"

class ExploreNodeRoot : public ExploreNode {
public:
	ExploreNodeRoot(Explore* explore);
	ExploreNodeRoot(Explore* explore,
					std::ifstream& save_file);
	~ExploreNodeRoot();

	void process() override;

	void save(std::ofstream& save_file) override;
};

#endif /* EXPLORE_NODE_ROOT_H */