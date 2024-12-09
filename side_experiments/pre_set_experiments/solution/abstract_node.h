#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>
#include <vector>

class AbstractExperiment;
class Scope;
class Solution;

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_SCOPE = 1;
const int NODE_TYPE_BRANCH = 2;

class AbstractNode {
public:
	int type;

	Scope* parent;
	int id;

	/**
	 * - simply don't allow experiments on both branches of a branch node
	 */
	AbstractExperiment* experiment;
	bool experiment_is_branch;

	double average_instances_per_run;

	virtual ~AbstractNode() {};

	virtual void save(std::ofstream& output_file) = 0;
	virtual void link(Solution* parent_solution) = 0;
	virtual void save_for_display(std::ofstream& output_file) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;
	int index;

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */