#ifndef EXPLORE_NODE_STATE_H
#define EXPLORE_NODE_STATE_H

#include "explore_node.h"

class ExploreNodeState : public ExploreNode {
public:
	int scope_index;
	std::vector<int> new_state_indexes;

	ExploreNodeState(Explore* explore,
					 int scope_index,
					 std::vector<int> new_state_indexes);
	ExploreNodeState(Explore* explore,
					 std::ifstream& save_file);
	~ExploreNodeState();

	void process() override;

	void save(std::ofstream& save_file) override;
};

#endif /* EXPLORE_NODE_STATE_H */