#ifndef EXPLORE_NODE_STATE_H
#define EXPLORE_NODE_STATE_H

class ExploreNodeState : public ExploreNode {
public:
	int scope_index;
	std::vector<int> new_state_indexes;

	ExploreNodeState(Explore* explore,
					 int scope_index,
					 std::vector<int> new_state_indexes);
	~ExploreNodeState();

	void process() override;
};

#endif /* EXPLORE_NODE_STATE_H */