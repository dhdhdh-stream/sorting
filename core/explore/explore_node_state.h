#ifndef EXPLORE_NODE_STATE_H
#define EXPLORE_NODE_STATE_H

class ExploreNodeState : public ExploreNode {
public:
	int scope_index;
	int new_state_index;

	void process() override;
};

#endif /* EXPLORE_NODE_STATE_H */