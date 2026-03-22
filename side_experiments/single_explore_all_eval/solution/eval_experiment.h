#ifndef EVAL_EXPERIMENT_H
#define EVAL_EXPERIMENT_H

#include <set>
#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class SolutionWrapper;

class EvalExperimentHistory;
class EvalExperimentState;
class EvalExperiment : public AbstractExperiment {
public:
	int state_iter;

	Scope* best_new_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;

	std::vector<Network*> new_networks;

	double existing_sum_scores;
	int existing_count;
	double new_sum_scores;
	int new_count;

	int curr_ramp;
	/**
	 * - simply init to 0 and fast fail
	 */

	double local_improvement;
	double global_improvement;

	EvalExperiment();
	~EvalExperiment();

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

	void backprop(double target_val,
				  EvalExperimentHistory* history,
				  SolutionWrapper* wrapper,
				  std::set<Scope*>& updated_scopes);

	void add(SolutionWrapper* wrapper);
};

class EvalExperimentHistory {
public:
	EvalExperiment* experiment;

	bool is_on;

	bool hit_branch;

	EvalExperimentHistory(EvalExperiment* experiment);
};

class EvalExperimentState : public AbstractExperimentState {
public:
	int step_index;

	EvalExperimentState(EvalExperiment* experiment);
};

#endif /* EVAL_EXPERIMENT_H */