#include "eval_node.h"

#include "network.h"

using namespace std;

EvalNode::EvalNode() {
	this->type = LOGIC_NODE_TYPE_EVAL;

	this->experiment = NULL;
}

EvalNode::~EvalNode() {
	delete this->network;
}

void EvalNode::save(ofstream& output_file) {
	this->network->save(output_file);

	output_file << this->weight << endl;
}

void EvalNode::load(ifstream& input_file) {
	this->network = new Network(input_file);

	string weight_line;
	getline(input_file, weight_line);
	this->weight = stod(weight_line);
}
