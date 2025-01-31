#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>
#include <vector>

class AbstractExperiment;
class Scope;

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_SCOPE = 1;
const int NODE_TYPE_BRANCH = 2;
const int NODE_TYPE_OBS = 3;

class AbstractNode {
public:
	int type;

	Scope* parent;
	int id;

	std::vector<int> ancestor_ids;
	/**
	 * - if both paths of BranchNode point to same node, add twice
	 */

	std::vector<AbstractExperiment*> experiments;

	virtual ~AbstractNode() {};

	virtual void save(std::ofstream& output_file) = 0;
	virtual void link() = 0;
	virtual void save_for_display(std::ofstream& output_file) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */