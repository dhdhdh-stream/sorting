#ifndef EXPLORE_NODE_IF_START_JUMP_H
#define EXPLORE_NODE_IF_START_JUMP_H

const int IF_START_JUMP_TYPE_END = 0;
const int IF_START_JUMP_TYPE_NORMAL = 1;

class ExploreNodeIfStartJump : public ExploreNode {
public:
	int if_start_jump_type;

	int jump_start_node_index;

	std::vector<int> new_path_node_indexes;

	// IF_START_JUMP_ENDING_TYPE_END
	int new_child_index;

	// OTHERWISE
	int modify_child_index;
	int new_start_node_index;
	int jump_end_node_index;
	int new_end_node_index;

	void process(Solution* solution) override;
};

#endif /* EXPLORE_NODE_IF_START_JUMP_H */