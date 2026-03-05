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

const int EVAL_EXPERIMENT_STATE_REFINE = 0;
const int EVAL_EXPERIMENT_STATE_INIT = 1;
const int EVAL_EXPERIMENT_STATE_RAMP = 2;
const int EVAL_EXPERIMENT_STATE_MEASURE = 3;

const int MEASURE_STATUS_N_A = 0;
const int MEASURE_STATUS_SUCCESS = 1;
const int MEASURE_STATUS_FAIL = 2;

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

	std::vector<Network*> new_networks;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;

	int num_original;
	int num_branch;

	std::vector<std::vector<double>> existing_obs_histories;
	std::vector<double> existing_target_val_histories;
	std::vector<std::vector<double>> new_obs_histories;
	std::vector<double> new_target_val_histories;

	int curr_ramp;

	int measure_status;

	int starting_experiment_iter;

	double local_improvement;
	double global_improvement;
	double score_standard_deviation;

	int result;

	std::vector<double> existing_scores;
	std::vector<double> new_scores;

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

	void refine_check_activate(AbstractNode* experiment_node,
							   std::vector<double>& obs,
							   SolutionWrapper* wrapper,
							   EvalExperimentHistory* history);
	void refine_backprop(double target_val,
						 EvalExperimentHistory* history,
						 SolutionWrapper* wrapper);

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