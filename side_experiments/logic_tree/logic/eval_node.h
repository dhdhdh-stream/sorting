#ifndef EVAL_NODE_H
#define EVAL_NODE_H

#include <fstream>

#include "abstract_logic_node.h"

class Network;

class EvalNode : public AbstractLogicNode {
public:
	Network* network;

	EvalNode();
	~EvalNode();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
};

#endif /* EVAL_NODE_H */