#ifndef EXPLORE_EXPERIMENT_H
#define EXPLORE_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class ExploreInstance;
class SolutionWrapper;

class ExploreExperiment : public AbstractExperiment {
public:
	int state_iter;

	ExploreInstance* curr_explore_instance;
	std::vector<ExploreInstance*> explore_instances;

	int total_count;
	double average_hits_per_run;

	ExploreExperiment(Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  SolutionWrapper* wrapper);

	void result_check_activate(AbstractNode* experiment_node,
							   bool is_branch,
							   SolutionWrapper* wrapper);
	void result_backprop(double target_val,
						 SolutionWrapper* wrapper);

	void check_activate(AbstractNode* experiment_node,
						bool is_branch,
						SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 bool& fetch_action,
						 SolutionWrapper* wrapper);
	void set_action(int action,
					SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);
	void backprop(double target_val,
				  SolutionWrapper* wrapper);

	void clean();
};

class ExploreExperimentHistory : public AbstractExperimentHistory {
public:
	int num_instances;

	int explore_index;

	std::vector<ScopeHistory*> stack_trace;

	ExploreExperimentHistory(ExploreExperiment* experiment);
};

class ExploreExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExploreExperimentState(ExploreExperiment* experiment);
};

#endif /* EXPLORE_EXPERIMENT_H */