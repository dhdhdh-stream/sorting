#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>

class Experiment;
class ExperimentRun;
class Run;
class Solution;
class Wrapper;

const int NODE_TYPE_START = 0;
const int NODE_TYPE_ACTION = 1;
const int NODE_TYPE_BRANCH = 2;
const int NODE_TYPE_END = 3;

class AbstractNode {
public:
	int type;

	int id;

	std::vector<int> ancestor_ids;
	/**
	 * - if both paths of BranchNode point to same node, add twice
	 */

	virtual ~AbstractNode() {};

	virtual void step(int& action,
					  bool& is_next,
					  Run* run) = 0;

	virtual void experiment_step(int& action,
								 bool& is_next,
								 ExperimentRun* run) = 0;
	virtual void experiment_step_start(ExperimentRun* run) = 0;

	virtual void save(std::ofstream& output_file,
					  Wrapper* wrapper) = 0;
	virtual void link(Wrapper* wrapper) = 0;
	virtual void save_for_display(std::ofstream& output_file) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	int index;

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */