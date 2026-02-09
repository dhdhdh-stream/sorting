#ifndef ABSTRACT_DECISION_TREE_NODE_H
#define ABSTRACT_DECISION_TREE_NODE_H

#include <fstream>
#include <vector>

class DecisionTree;

const int DECISION_TREE_NODE_TYPE_PREVIOUS = -1;
const int DECISION_TREE_NODE_TYPE_LINEAR = 0;
const int DECISION_TREE_NODE_TYPE_NETWORK = 1;

class AbstractDecisionTreeNode {
public:
	int type;

	int id;

	bool has_split;
	int obs_index;
	int rel_obs_index;
	int split_type;
	double split_target;
	double split_range;

	int original_node_id;
	AbstractDecisionTreeNode* original_node;
	int branch_node_id;
	AbstractDecisionTreeNode* branch_node;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> previous_val_histories;
	std::vector<double> target_val_histories;

	virtual ~AbstractDecisionTreeNode() {};

	virtual double activate(std::vector<double>& obs,
							double previous_val) = 0;

	virtual void update(DecisionTree* decision_tree) = 0;

	virtual void save(std::ofstream& output_file) = 0;
	virtual void link(DecisionTree* decision_tree) = 0;
};

#endif /* ABSTRACT_DECISION_TREE_NODE_H */