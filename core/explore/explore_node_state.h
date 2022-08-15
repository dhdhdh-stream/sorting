#ifndef EXPLORE_NODE_STATE_H
#define EXPLORE_NODE_STATE_H

class ExploreNodeState : public ExploreNode {
public:
	SolutionNode* scope;
	int new_state_index;

	void process(Solution* solution) override;
};

#endif /* EXPLORE_NODE_STATE_H */