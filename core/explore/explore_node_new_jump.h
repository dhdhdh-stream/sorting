#ifndef EXPLORE_NODE_NEW_JUMP_H
#define EXPLORE_NODE_NEW_JUMP_H

#include "explore_node.h"

class ExploreNodeNewJump : public ExploreNode {
public:
	int jump_start_non_inclusive_index;
	int jump_start_inclusive_index;
	int jump_end_inclusive_index;
	int jump_end_non_inclusive_index;

	int new_start_node_index;
	int new_end_node_index;

	std::vector<int> new_path_node_indexes;

	ExploreNodeNewJump(Explore* explore,
					   int jump_start_non_inclusive_index,
					   int jump_start_inclusive_index,
					   int jump_end_inclusive_index,
					   int jump_end_non_inclusive_index,
					   int new_start_node_index,
					   int new_end_node_index,
					   std::vector<int> new_path_node_indexes);
	ExploreNodeNewJump(Explore* explore,
					   std::ifstream& save_file);
	~ExploreNodeNewJump();

	void process() override;

	void save(std::ofstream& save_file) override;
};

#endif /* EXPLORE_NODE_NEW_JUMP_H */