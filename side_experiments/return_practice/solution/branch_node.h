#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <vector>

#include "abstract_node.h"

class Experiment;
class Network;

class BranchNode : public AbstractNode {
public:
	/**
	 * - need to crunch world modeling down into simple equation
	 *   - otherwise, expensive when many branches
	 */
	std::vector<int> input_indexes;
	Network* original_network;
	Network* branch_network;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	std::vector<std::vector<double>> original_state_history;
	std::vector<std::vector<double>> branch_state_history;

	Experiment* original_experiment;
	Experiment* branch_experiment;



	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 ExperimentRun& run);
	void predict_step(PredictRun& run);


};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	BranchNodeHistory(BranchNode* node);
};

#endif /* BRANCH_NODE_H */