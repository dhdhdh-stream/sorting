#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

class BranchNode : public AbstractNode {
public:
	std::vector<int> branch_scope_context;
	std::vector<int> branch_node_context;
	
};

#endif /* BRANCH_NODE_H */