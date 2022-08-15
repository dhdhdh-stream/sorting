#ifndef EXPLORE_NODE_GOTO_H
#define EXPLORE_NODE_GOTO_H

class ExploreNodeGoto {
public:
	int node_index;
	int goto_index;

	int new_start_node_index;
	int new_end_node_index;

	std::vector<int> new_path_node_indexes;

	void process(Solution* solution) override;
};

#endif /* EXPLORE_NODE_GOTO_H */