/**
 * Branch size always 2.
 * (So essentially saying there's no continuous generalization, with that effect instead achieved through loops.)
 */

#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <vector>

#include "abstract_node.h"
#include "run_helper.h"
#include "state_network.h"

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	std::vector<int> branch_scope_context;
	std::vector<int> branch_node_context;
	bool branch_is_pass_through;
	StateNetwork* branch_score_network;
	// Note: eventually, ideally, right branch will have score 0.0 while other will be negative
	int branch_exit_depth;
	int branch_next_node_id;

	StateNetwork* original_score_network;
	int original_next_node_id;

	BranchNode(std::vector<int> branch_scope_context,
			   std::vector<int> branch_node_context,
			   bool branch_is_pass_through,
			   StateNetwork* branch_score_network,
			   int branch_exit_depth,
			   int branch_next_node_id,
			   StateNetwork* original_score_network,
			   int original_next_node_id);
	BranchNode(std::ifstream& input_file,
			   int scope_id,
			   int scope_index);
	~BranchNode();

	void activate(std::vector<double>& state_vals,
				  double& predicted_score,
				  double& scale_factor,
				  std::vector<int>& scope_context,
				  std::vector<int>& node_context,
				  int& exit_depth,
				  int& exit_node_id,
				  BranchNodeHistory* history);
	void backprop(std::vector<double>& state_errors,
				  double target_val,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  BranchNodeHistory* history);

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;
	StateNetworkHistory* score_network_history;
	double score_network_update;

	BranchNodeHistory(BranchNode* node,
					  int scope_index);
	~BranchNodeHistory();
};

#endif /* BRANCH_NODE_H */