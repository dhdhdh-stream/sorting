#ifndef EXPLORE_NODE_STATE_H
#define EXPLORE_NODE_STATE_H

class ExploreNodeState : public ExploreNode {
public:
	int scope_index;
	vector<int> new_state_indexes;

	void process() override;
};

#endif /* EXPLORE_NODE_STATE_H */