#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_SCOPE = 1;
const int NODE_TYPE_BRANCH = 2;
const int NODE_TYPE_BRANCH_STUB = 3;
const int NODE_TYPE_EXIT = 4;

class BranchExperiment;

class AbstractNode {
public:
	int type;

	int id;

	virtual ~AbstractNode() {};
	virtual void save(std::ofstream& output_file) = 0;
	virtual void save_for_display(std::ofstream& output_file) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	BranchExperimentHistory* branch_experiment_history;

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */