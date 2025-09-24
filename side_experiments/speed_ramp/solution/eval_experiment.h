#ifndef EVAL_EXPERIMENT_H
#define EVAL_EXPERIMENT_H

#include <set>
#include <vector>

#include "abstract_experiment.h"
#include "input.h"

class AbstractNode;
class Network;
class Scope;
class SolutionWrapper;

const int EVAL_EXPERIMENT_STATE_INITIAL = 0;
const int EVAL_EXPERIMENT_STATE_RAMP = 1;

const int EVAL_RESULT_NA = 0;
const int EVAL_RESULT_FAIL = 1;
const int EVAL_RESULT_SUCCESS = 2;

class EvalExperimentHistory;
class EvalExperimentState;
class EvalExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int num_fail;
	/**
	 * - don't rely on t-test to drive exploration
	 *   - can gradually lead to worse and worse results
	 *   - doesn't always enable where a small penalty leads to greater score anyways
	 */

	double select_percentage;

	double new_average_score;
	std::vector<Input> new_inputs;
	std::vector<double> new_input_averages;
	std::vector<double> new_input_standard_deviations;
	std::vector<double> new_weights;
	std::vector<Input> new_network_inputs;
	Network* new_network;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;

	int curr_ramp;

	int result;

	std::vector<double> existing_scores;
	std::vector<double> new_scores;

	EvalExperiment();
	~EvalExperiment();

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
				  EvalExperimentHistory* history,
				  SolutionWrapper* wrapper,
				  std::set<Scope*>& updated_scopes);

	void initial_backprop(double target_val,
						  EvalExperimentHistory* history,
						  SolutionWrapper* wrapper);

	void ramp_backprop(double target_val,
					   EvalExperimentHistory* history,
					   SolutionWrapper* wrapper,
					   std::set<Scope*>& updated_scopes);

	void clean_inputs(Scope* scope,
					  int node_id);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);

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