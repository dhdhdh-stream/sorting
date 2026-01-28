#ifndef ABSTRACT_LOGIC_NODE_H
#define ABSTRACT_LOGIC_NODE_H

#include <fstream>
#include <vector>

class LogicExperiment;

const int LOGIC_NODE_TYPE_SPLIT = 0;
const int LOGIC_NODE_TYPE_EVAL = 1;

class AbstractLogicNode {
public:
	int type;

	int id;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;
	std::vector<double> existing_predicted_histories;

	virtual ~AbstractLogicNode() {};

	virtual void save(std::ofstream& output_file) = 0;
};

#endif /* ABSTRACT_LOGIC_NODE_H */