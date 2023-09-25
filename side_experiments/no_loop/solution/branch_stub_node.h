#ifndef BRANCH_STUB_NODE_H
#define BRANCH_STUB_NODE_H

class BranchStubNode : public AbstractNode {
public:
	bool was_branch;

	std::vector<int> local_state_ids;
	std::vector<int> obs_ids;
	std::vector<State*> states;
	std::vector<int> network_indexes;

	int next_node_id;
	
};

#endif /* BRANCH_STUB_NODE_H */