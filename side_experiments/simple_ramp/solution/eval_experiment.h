#ifndef EVAL_EXPERIMENT_H
#define EVAL_EXPERIMENT_H

#include <set>
#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class ObsNode;
class Scope;
class SolutionWrapper;

const int EVAL_EXPERIMENT_STATE_INIT = 0;
const int EVAL_EXPERIMENT_STATE_RAMP = 1;

const int EVAL_RESULT_FAIL = 0;
const int EVAL_RESULT_SUCCESS = 1;

class EvalExperimentHistory;
class EvalExperimentState;
class EvalExperiment : public AbstractExperiment {
public:
	ObsNode* node_context;
	AbstractNode* exit_next_node;

	int state;
	int state_iter;
	int num_fail;
	/**
	 * - don't rely on t-test to drive exploration
	 *   - can gradually lead to worse and worse results
	 *   - doesn't always enable where a small penalty leads to greater score anyways
	 */

	Network* new_network;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;

	int curr_ramp;

	int result;

	double existing_sum_scores;
	int existing_count;
	double new_sum_scores;
	int new_count;

	EvalExperiment();
	~EvalExperiment();

	void check_activate(AbstractNode* experiment_node,
						std::vector<double>& obs,
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
	bool is_on;

	EvalExperimentHistory(EvalExperiment* experiment);
};

class EvalExperimentState : public AbstractExperimentState {
public:
	int step_index;

	EvalExperimentState(EvalExperiment* experiment);
};

#endif /* EVAL_EXPERIMENT_H */