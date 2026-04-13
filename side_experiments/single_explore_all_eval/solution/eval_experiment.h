#ifndef EVAL_EXPERIMENT_H
#define EVAL_EXPERIMENT_H

#include <set>
#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
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
	bool is_damage;

	int state;
	int state_iter;

	Scope* best_new_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;

	std::vector<Network*> new_networks;

	std::vector<double> clean_existing_scores;
	std::vector<double> clean_new_scores;
	std::vector<double> damage_existing_scores;
	std::vector<double> damage_new_scores;

	int curr_ramp;
	/**
	 * - simply init to 0 and fast fail
	 */
	int measure_status;

	int starting_iter;

	double clean_local_improvement;
	double clean_global_improvement;
	double damage_local_improvement;
	double damage_global_improvement;

	EvalExperiment(SolutionWrapper* wrapper);
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

	void ramp_check_activate(SolutionWrapper* wrapper);
	void ramp_step(std::vector<double>& obs,
				   int& action,
				   bool& is_next,
				   bool& fetch_action,
				   SolutionWrapper* wrapper);
	void ramp_exit_step(SolutionWrapper* wrapper);
	void ramp_backprop(double target_val,
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