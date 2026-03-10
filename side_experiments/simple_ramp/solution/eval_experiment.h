/**
 * - don't explicitly worry about passthrough
 *   - 100% vs 90% branch not significantly different in terms of fracturing
 */

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

const int EVAL_EXPERIMENT_STATE_RAMP = 0;
const int EVAL_EXPERIMENT_STATE_MEASURE = 1;

const int MEASURE_STATUS_N_A = 0;
const int MEASURE_STATUS_SUCCESS = 1;
const int MEASURE_STATUS_FAIL = 2;

class EvalExperimentHistory;
class EvalExperimentState;
class EvalExperiment : public AbstractExperiment {
public:
	ObsNode* node_context;
	AbstractNode* exit_next_node;

	int state;
	int state_iter;
	/**
	 * - don't rely on negative t-test to drive exploration
	 *   - can gradually lead to worse and worse results
	 *   - doesn't always enable where a small penalty leads to greater score anyways
	 */

	std::vector<Network*> new_networks;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;

	int num_original;
	int num_branch;

	int starting_iter;

	std::vector<double> existing_scores;
	std::vector<double> new_scores;

	int curr_ramp;
	/**
	 * - simply init to 0 and fast fail
	 */
	int measure_status;

	double local_improvement;
	double global_improvement;
	double score_standard_deviation;

	EvalExperiment(SolutionWrapper* wrapper);
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

	void ramp_check_activate(AbstractNode* experiment_node,
							 std::vector<double>& obs,
							 SolutionWrapper* wrapper,
							 EvalExperimentHistory* history);
	void ramp_backprop(double target_val,
					   EvalExperimentHistory* history,
					   SolutionWrapper* wrapper,
					   std::set<Scope*>& updated_scopes);

	void add(SolutionWrapper* wrapper);
};

class EvalExperimentHistory {
public:
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