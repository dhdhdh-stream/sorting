#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <vector>

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

	bool branch_is_pass_through;

	ScoreNetwork* branch_score_network;
	ScoreNetwork* branch_misguess_network;
	double branch_score_update;
	double branch_misguess_update;
	int branch_next_node_id;

	ScoreNetwork* original_score_network;
	ScoreNetwork* original_misguess_network;
	double original_score_update;
	double original_misguess_update;
	int original_next_node_id;

	int remeasure_counter;

};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;


};

class RemeasureBranchNodeHistory {
public:
	BranchNode* node;

	double running_average_score_snapshot;
	double running_average_misguess_snapshot;
	double scale_factor_snapshot;

	ScoreNetworkHistory* score_network_history;
	double score_network_output;
	ScoreNetworkHistory* misguess_network_history;
	double misguess_network_output;


};

#endif /* BRANCH_NODE_H */