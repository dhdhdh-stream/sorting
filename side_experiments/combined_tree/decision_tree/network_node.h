#ifndef NETWORK_NODE_H
#define NETWORK_NODE_H

#include "abstract_decision_tree_node.h"

class Network;

class NetworkNode : public AbstractDecisionTreeNode {
public:
	std::vector<int> input_indexes;
	/**
	 * - num network inputs is 1 + input_indexes.size()
	 *   - with 1st input being previous val
	 */
	Network* network;

	NetworkNode();
	~NetworkNode();

	double activate(std::vector<double>& obs,
					double previous_val);

	void update(DecisionTree* decision_tree);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(DecisionTree* decision_tree);

	void copy_from(NetworkNode* original);
};

#endif /* NETWORK_NODE_H */