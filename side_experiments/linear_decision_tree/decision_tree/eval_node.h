#ifndef EVAL_NODE_H
#define EVAL_NODE_H

#include <fstream>

#include "abstract_decision_tree_node.h"

class Network;

const int EVAL_NODE_NUM_INPUTS = 10;

class EvalNode : public AbstractDecisionTreeNode {
public:
	double constant;
	std::vector<int> input_indexes;
	std::vector<double> input_weights;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;

	EvalNode();

	double activate(std::vector<double>& obs);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);

	void copy_from(EvalNode* original);
};

#endif /* EVAL_NODE_H */