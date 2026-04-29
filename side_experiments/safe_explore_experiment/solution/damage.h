#ifndef DAMAGE_H
#define DAMAGE_H

#include <vector>

#include "abstract_experiment.h"

class Damage : public AbstractExperiment {
public:
	bool is_dangerous;
	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	Damage(AbstractNode* node_context,
		   bool is_branch,
		   SolutionWrapper* wrapper);
	~Damage();

	void experiment_check_activate(AbstractNode* experiment_node,
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

	void result_check_activate(AbstractNode* experiment_node,
							   std::vector<double>& obs,
							   SolutionWrapper* wrapper);
	void result_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 bool& fetch_action,
					 SolutionWrapper* wrapper);
	void result_set_action(int action,
						   SolutionWrapper* wrapper);
	void result_exit_step(SolutionWrapper* wrapper);
};

class DamageState : public AbstractExperimentState {
public:
	int step_index;

	DamageState(Damage* damage);
};

#endif /* DAMAGE_H */