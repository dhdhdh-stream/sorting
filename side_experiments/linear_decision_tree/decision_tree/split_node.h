#ifndef SPLIT_NODE_H
#define SPLIT_NODE_H

#include <fstream>

#include "abstract_decision_tree_node.h"

class DecisionTree;

const int SPLIT_TYPE_GREATER = 0;
const int SPLIT_TYPE_GREATER_EQUAL = 1;
const int SPLIT_TYPE_LESSER = 2;
const int SPLIT_TYPE_LESSER_EQUAL = 3;
const int SPLIT_TYPE_WITHIN = 4;
const int SPLIT_TYPE_WITHIN_EQUAL = 5;
const int SPLIT_TYPE_WITHOUT = 6;
const int SPLIT_TYPE_WITHOUT_EQUAL = 7;
const int SPLIT_TYPE_REL_GREATER = 8;
const int SPLIT_TYPE_REL_GREATER_EQUAL = 9;
const int SPLIT_TYPE_REL_WITHIN = 10;
const int SPLIT_TYPE_REL_WITHIN_EQUAL = 11;
const int SPLIT_TYPE_REL_WITHOUT = 12;
const int SPLIT_TYPE_REL_WITHOUT_EQUAL = 13;

bool is_match_helper(std::vector<double>& obs,
					 int obs_index,
					 int rel_obs_index,
					 int split_type,
					 double split_target,
					 double split_range);

class SplitNode : public AbstractDecisionTreeNode {
public:
	int obs_index;
	int rel_obs_index;
	int split_type;
	double split_target;
	double split_range;

	int original_node_id;
	AbstractDecisionTreeNode* original_node;
	int branch_node_id;
	AbstractDecisionTreeNode* branch_node;

	SplitNode();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(DecisionTree* decision_tree);

	void copy_from(SplitNode* original);
};

#endif /* SPLIT_NODE_H */