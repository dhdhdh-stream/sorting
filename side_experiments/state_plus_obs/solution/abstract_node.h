#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>
#include <vector>

#include <Eigen/Dense>

class AbstractExperiment;
class InitNetwork;
class Scope;
class ScopeHistory;
class Solution;
class SolutionWrapper;
class TrainScopeHistory;

const int NODE_TYPE_NOOP = 0;
const int NODE_TYPE_ACTION = 1;
const int NODE_TYPE_SCOPE = 2;
const int NODE_TYPE_BRANCH = 3;

class AbstractNodeHistory;
class AbstractNode {
public:
	int type;

	Scope* parent;
	int id;

	std::vector<int> ancestor_ids;
	/**
	 * - if both paths of BranchNode point to same node, add twice
	 */

	virtual ~AbstractNode() {};

	virtual void step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper) = 0;

	virtual void experiment_step(std::vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) = 0;

	virtual void train_step(AbstractNodeHistory* history,
							bool allow_drop,
							Eigen::VectorXf& state,
							TrainScopeHistory* train_scope_history) = 0;

	virtual void save(std::ofstream& output_file) = 0;
	virtual void link(Solution* parent_solution) = 0;
	virtual void save_for_display(std::ofstream& output_file) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	virtual ~AbstractNodeHistory() {};
};

class TrainAbstractNodeHistory {
public:
	AbstractNode* node;

	virtual ~TrainAbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */