#ifndef BRANCH_STUB_NODE_H
#define BRANCH_STUB_NODE_H

class BranchStubNode : public AbstractNode {
public:
	bool was_branch;

	std::vector<bool> state_is_local;
	std::vector<int> state_ids;
	std::vector<State*> state_defs;
	std::vector<int> state_network_indexes;

	int next_node_id;

	
};

class BranchStubNodeHistory : public AbstractNodeHistory {
public:
	BranchStubNodeHistory(BranchStubNode* node);
};

#endif /* BRANCH_STUB_NODE_H */