#include "eval_node.h"

#include "network.h"

using namespace std;

EvalNode::EvalNode() {
	this->type = DECISION_TREE_NODE_TYPE_EVAL;
}

EvalNode::~EvalNode() {
	delete this->network;
}

void EvalNode::save(ofstream& output_file) {
	this->network->save(output_file);
}

void EvalNode::load(ifstream& input_file) {
	this->network = new Network(input_file);
}

void EvalNode::copy_from(EvalNode* original) {
	this->network = new Network(original->network);

	this->obs_histories = original->obs_histories;
	this->target_val_histories = original->target_val_histories;
}
