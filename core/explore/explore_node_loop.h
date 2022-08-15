#ifndef EXPLORE_NODE_LOOP_H
#define EXPLORE_NODE_LOOP_H

class ExploreNodeLoop : public ExploreNode {
public:
	int loop_start_node_index;
	int new_start_node_index;

	int loop_end_node_index;
	int new_end_node_index;

	std::vector<int> new_path_node_indexes;

	void process(Solution* solution) override;
};

#endif /* EXPLORE_NODE_LOOP_H */