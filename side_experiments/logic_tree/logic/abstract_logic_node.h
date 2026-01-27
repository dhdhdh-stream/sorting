#ifndef ABSTRACT_LOGIC_NODE_H
#define ABSTRACT_LOGIC_NODE_H

#include <fstream>

class LogicExperiment;

const int LOGIC_NODE_TYPE_SPLIT = 0;
const int LOGIC_NODE_TYPE_EVAL = 1;

class AbstractLogicNode {
public:
	int type;

	int id;

	double weight;

	LogicExperiment* experiment;

	virtual ~AbstractLogicNode() {};

	virtual void save(std::ofstream& output_file) = 0;
};

#endif /* ABSTRACT_LOGIC_NODE_H */