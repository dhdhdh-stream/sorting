#ifndef EVAL_NODE_H
#define EVAL_NODE_H

#include <fstream>

#include "abstract_decision_tree_node.h"

class Network;

#if defined(MDEBUG) && MDEBUG
const int EVAL_NUM_TRAIN_SAMPLES = 40;
const int EVAL_NUM_TEST_SAMPLES = 10;
#else
const int EVAL_NUM_TRAIN_SAMPLES = 800;
const int EVAL_NUM_TEST_SAMPLES = 200;
#endif /* MDEBUG */
const int EVAL_NUM_TOTAL_SAMPLES = EVAL_NUM_TRAIN_SAMPLES + EVAL_NUM_TEST_SAMPLES;

class EvalNode : public AbstractDecisionTreeNode {
public:
	Network* network;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;

	EvalNode();
	~EvalNode();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);

	void copy_from(EvalNode* original);
};

#endif /* EVAL_NODE_H */