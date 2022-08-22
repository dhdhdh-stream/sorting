#ifndef EXPLORE_NODE_APPEND_JUMP_H
#define EXPLORE_NODE_APPEND_JUMP_H

class ExploreNodeAppendJump : public ExploreNode {
public:
	int jump_start_node_index;

	int new_child_index;

	std::vector<int> new_path_node_indexes;

	ExploreNodeAppendJump(Explore* explore,
						  int jump_start_node_index,
						  int new_child_index,
						  std::vector<int> new_path_node_indexes);
	~ExploreNodeAppendJump();

	void process() override;
};

#endif /* EXPLORE_NODE_APPEND_JUMP_H */