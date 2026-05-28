#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>

class Experiment;
class ExperimentRun;
class PredictRun;
class Run;
class Solution;
class Wrapper;

const int NODE_TYPE_OBS = 0;
const int NODE_TYPE_ACTION = 1;
const int NODE_TYPE_BRANCH = 2;

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
	virtual void predict_step(PredictRun* run) = 0;

	virtual void save(std::ofstream& output_file,
					  Wrapper* wrapper) = 0;
	virtual void link(Wrapper* wrapper) = 0;
	virtual void save_for_display(std::ofstream& output_file) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */