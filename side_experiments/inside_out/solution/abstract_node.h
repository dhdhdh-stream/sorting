#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>
#include <vector>

class AbstractExperiment;
class Confusion;
class Scope;
class Solution;
class SolutionWrapper;

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

	bool is_init;

	AbstractExperiment* experiment;

	Confusion* confusion;

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
	virtual void measure_update() = 0;

	virtual void new_scope_clean() = 0;
	virtual void new_scope_measure_update() = 0;

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