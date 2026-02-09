#ifndef LINEAR_NODE_H
#define LINEAR_NODE_H

#include "abstract_decision_tree_node.h"

class LinearNode : public AbstractDecisionTreeNode {
public:
	double constant;
	std::vector<int> input_indexes;
	std::vector<double> input_weights;
	double previous_weight;

	LinearNode();

	double activate(std::vector<double>& obs,
					double previous_val);

	void update(DecisionTree* decision_tree);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(DecisionTree* decision_tree);

	void copy_from(LinearNode* original);
};

#endif /* LINEAR_NODE_H */