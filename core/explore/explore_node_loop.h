#ifndef EXPLORE_NODE_LOOP_H
#define EXPLORE_NODE_LOOP_H

#include "explore_node.h"

class ExploreNodeLoop : public ExploreNode {
public:
	int loop_start_non_inclusive_index;
	int loop_start_inclusive_index;
	int loop_end_inclusive_index;
	int loop_end_non_inclusive_index;

	int new_start_node_index;
	int new_end_node_index;

	ExploreNodeLoop(Explore* explore,
					int loop_start_non_inclusive_index,
					int loop_start_inclusive_index,
					int loop_end_inclusive_index,
					int loop_end_non_inclusive_index,
					int new_start_node_index,
					int new_end_node_index);
	ExploreNodeLoop(Explore* explore,
					std::ifstream& save_file);
	~ExploreNodeLoop();

	void process() override;

	void save(std::ofstream& save_file) override;
};

#endif /* EXPLORE_NODE_LOOP_H */