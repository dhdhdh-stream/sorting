#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>
#include <vector>

class AbstractExperiment;
class Scope;
class Solution;
class SolutionWrapper;

const int NODE_TYPE_START = 0;
const int NODE_TYPE_ACTION = 1;
const int NODE_TYPE_SCOPE = 2;
const int NODE_TYPE_BRANCH = 3;
const int NODE_TYPE_OBS = 4;

class AbstractNode {
public:
	int type;

	Scope* parent;
	int id;

	std::vector<int> impacted_factors;

	std::vector<int> ancestor_ids;
	/**
	 * - if both paths of BranchNode point to same node, add twice
	 */

	AbstractExperiment* experiment;

	virtual ~AbstractNode() {};

	virtual void step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper) = 0;

	virtual void experiment_step(std::vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) = 0;

	virtual void clean() = 0;
	virtual void measure_update(int total_count) = 0;

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