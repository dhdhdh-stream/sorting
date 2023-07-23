/**
 * - score networks for the wrong branch will not be accurate
 *   - over time, correct branch should have a score of 0.0
 *     - wrong branch will have a score of slightly less
 *       - but should still be good enough to make decisions
 * 
 * - don't update predicted_score with branch networks
 * 
 * - on reuse, on explore, hope that lucky state enables even small differences to be meaningful
 *   - then on experiment, both branches will initially be near 0.0, so will likely both be tried and re-trained appropriately
 *     - backprop error signals
 *       - (even though scores will be near 0.0, demand for state can be strong)
 */

#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

class BranchNode : public AbstractNode {
public:
	std::vector<int> branch_scope_context;
	std::vector<int> branch_node_context;
	bool branch_is_pass_through;
	StateNetwork* branch_score_network;
	int branch_next_node_id;

	StateNetwork* original_score_network;
	int original_next_node_id;

	double branch_weight;



	void activate(std::vector<ForwardContextLayer>& context,
				  RunHelper& run_helper,
				  BranchNodeHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  RunHelper& run_helper,
				  BranchNodeHistory* history);

};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch; // also used to signal outside

	std::vector<double> state_vals_snapshot;
	ScoreNetworkHistory* score_network_history;
	double score_network_output;


}

#endif /* BRANCH_NODE_H */