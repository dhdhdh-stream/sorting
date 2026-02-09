#ifndef PREVIOUS_NODE_H
#define PREVIOUS_NODE_H

#include "abstract_decision_tree_node.h"

class PreviousNode : public AbstractDecisionTreeNode {
public:
	PreviousNode();

	double activate(std::vector<double>& obs,
					double previous_val);

	void update(DecisionTree* decision_tree);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(DecisionTree* decision_tree);

	void copy_from(PreviousNode* original);
};

#endif /* PREVIOUS_NODE_H */