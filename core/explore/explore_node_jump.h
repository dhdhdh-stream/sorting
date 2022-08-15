#ifndef EXPLORE_NODE_JUMP_H
#define EXPLORE_NODE_JUMP_H

class ExploreNodeJump : public ExploreNode {
public:
	int jump_start_node_index;
	int jump_end_node_index;

	std::vector<int> new_path_node_indexes;

	int new_start_node_index;
	// first 2 children on
	int new_end_node_index;

	void process(Solution* solution) override;
};

#endif /* EXPLORE_NODE_JUMP_H */