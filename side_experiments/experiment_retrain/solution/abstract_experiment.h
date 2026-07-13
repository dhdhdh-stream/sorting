#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include <utility>
#include <vector>

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class AbstractExperimentHistory;
class AbstractExperiment {
public:
	Scope* scope_context;
	AbstractNode* node_context;
	bool is_branch;
	AbstractNode* exit_next_node;

	virtual ~AbstractExperiment() {};

	virtual void experiment_check_activate(std::vector<double>& obs,
										   SolutionWrapper* wrapper) = 0;
	virtual void experiment_step(std::vector<double>& obs,
								 int& action,
								 bool& is_next,
								 bool& fetch_action,
								 SolutionWrapper* wrapper) = 0;
	virtual void set_action(int action,
							SolutionWrapper* wrapper) = 0;
	virtual void experiment_step_callback(std::vector<double>& obs,
										  SolutionWrapper* wrapper) = 0;
	virtual void experiment_exit_step(SolutionWrapper* wrapper) = 0;
};

class AbstractExperimentState {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentState() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */