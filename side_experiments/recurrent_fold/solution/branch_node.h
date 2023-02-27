#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

class BranchNode {
public:
	Network* combined_score_network;	// takes context specific branches into account

	std::vector<int> scope_context;
	std::vector<int> node_context;
	std::vector<Network*> score_networks;
	// Note: eventually, ideally, right branch will have score 0.0 while others will be negative
	std::vector<int> next_indexes;	// index for ending scope
	std::vector<int> num_travelled;	// if there's recursion, scores may be inaccurate, so ease in to new branch
	// TODO: track when last travelled, and delete unused

	// TODO: adjust explore weight scale factor based on the branch

	// Note: for explore with loops, don't have to worry about timing, as explore weight will randomize it naturally

	// explore input initialized from start of solution
};

#endif /* BRANCH_NODE_H */