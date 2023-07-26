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

#include <vector>

#include "abstract_node.h"
#include "backward_context_layer.h"
#include "forward_context_layer.h"
#include "run_helper.h"
#include "score_network.h"

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	std::vector<int> branch_scope_context;
	std::vector<int> branch_node_context;
	/**
	 * - last layer of context doesn't matter
	 *   - scope id will always match, and node id meaningless
	 */
	bool branch_is_pass_through;
	ScoreNetwork* branch_score_network;
	ScoreNetwork* branch_misguess_network;
	/**
	 * - just use global misguess instead of instance misguess
	 *   - instance is sharper but requires a lot of effort to track
	 */
	int branch_next_node_id;

	ScoreNetwork* original_score_network;
	ScoreNetwork* original_misguess_network;
	int original_next_node_id;

	/**
	 * - to help create reasonable sequences for explore
	 *   - cheaper than saving past sequences
	 */
	double branch_weight;

	BranchNode(Scope* parent,
			   int id,
			   std::vector<int> branch_scope_context,
			   std::vector<int> branch_node_context,
			   bool branch_is_pass_through,
			   ScoreNetwork* branch_score_network,
			   ScoreNetwork* branch_misguess_network,
			   int branch_next_node_id,
			   ScoreNetwork* original_score_network,
			   ScoreNetwork* original_misguess_network,
			   int original_next_node_id);
	BranchNode(std::ifstream& input_file,
			   Scope* parent,
			   int id);
	~BranchNode();

	void activate(std::vector<ForwardContextLayer>& context,
				  RunHelper& run_helper,
				  BranchNodeHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  RunHelper& run_helper,
				  BranchNodeHistory* history);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch; // also used to signal outside

	std::vector<double> state_vals_snapshot;
	ScoreNetworkHistory* score_network_history;
	ScoreNetworkHistory* misguess_network_history;
	double score_network_output;
	double misguess_network_output;

	BranchNodeHistory(BranchNode* node);
	~BranchNodeHistory();
}

#endif /* BRANCH_NODE_H */