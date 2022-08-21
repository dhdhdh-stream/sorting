#ifndef EXPLORE_NODE_LOOP_H
#define EXPLORE_NODE_LOOP_H

class ExploreNodeLoop : public ExploreNode {
public:
	int loop_start_non_inclusive_index;
	int loop_start_inclusive_index;
	int loop_end_inclusive_index;
	int loop_end_non_inclusive_index;

	int new_start_node_index;
	int new_end_node_index;

	void process() override;
};

#endif /* EXPLORE_NODE_LOOP_H */