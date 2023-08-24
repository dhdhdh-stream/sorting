#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <vector>

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

	std::vector<int> backfill_start_layer;
	std::vector<int> backfill_state_index;

	bool branch_is_pass_through;

	ScoreNetwork* branch_score_network;
	ScoreNetwork* branch_misguess_network;
	int branch_next_node_id;

	ScoreNetwork* original_score_network;
	ScoreNetwork* original_misguess_network;
	int original_next_node_id;

	/**
	 * - to help create reasonable sequences for explore
	 *   - cheaper than saving past sequences
	 */
	double branch_weight;

	int remeasure_counter;

};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;	// also used to signal outside

	double predicted_score_snapshot;
	double scale_factor_snapshot;
	ScoreNetworkHistory* score_network_history;
	ScoreNetworkHistory* misguess_network_history;
	double score_network_output;
	double misguess_network_output;


};

#endif /* BRANCH_NODE_H */