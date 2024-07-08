#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

class BranchNode : public AbstractNode {
public:
	int analyze_size;
	Network* network;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	
};

#endif /* BRANCH_NODE_H */