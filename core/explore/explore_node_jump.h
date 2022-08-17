#ifndef EXPLORE_NODE_JUMP_H
#define EXPLORE_NODE_JUMP_H

class ExploreNodeJump : public ExploreNode {
public:
	int jump_start_node_index;
	int jump_end_node_index;

	// assuming simple path
	std::vector<int> new_path_node_indexes;

	int new_start_node_index;
	int new_end_node_index;

	void process() override;
};

#endif /* EXPLORE_NODE_JUMP_H */