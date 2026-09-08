#ifndef EXPLORE_H
#define EXPLORE_H

#include "abstract_experiment.h"

class Explore : public AbstractExperiment {
public:
	std::vector<int> step_types;
	std::vector<int> indexes;

	int num_instances_until_target;

	Explore(Scope* scope_context,
			AbstractNode* node_context,
			bool is_branch,
			AbstractNode* exit_next_node);
	~Explore();

	void experiment_check_activate(std::vector<double>& obs,
								   SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 bool& fetch_action,
						 SolutionWrapper* wrapper);
	void set_action(int action,
					SolutionWrapper* wrapper);
	void experiment_step_callback(std::vector<double>& obs,
								  SolutionWrapper* wrapper);
	void experiment_exit_step(std::vector<double>& obs,
							  SolutionWrapper* wrapper);
};

class ExploreHistory : public AbstractExperimentHistory {
public:
	bool has_explore;

	ExploreHistory(Explore* explore);
};

class ExploreState : public AbstractExperimentState {
public:
	int step_index;

	ExploreState(Explore* explore);
};

#endif /* EXPLORE_H */