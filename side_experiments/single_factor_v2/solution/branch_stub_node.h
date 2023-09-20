#ifndef BRANCH_STUB_NODE_H
#define BRANCH_STUB_NODE_H

class BranchStubNode : public AbstractNode {
public:
	bool was_branch;

	std::vector<int> state_ids;
	std::vector<StateNetwork*> state_networks;

	int next_node_id;
	
};

#endif /* BRANCH_STUB_NODE_H */