#ifndef PASS_THROUGH_EXPERIMENT_H
#define PASS_THROUGH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Scope;
class SolutionWrapper;

const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int PASS_THROUGH_EXPERIMENT_STATE_C1 = 1;
const int PASS_THROUGH_EXPERIMENT_STATE_C2 = 2;
const int PASS_THROUGH_EXPERIMENT_STATE_C3 = 3;
const int PASS_THROUGH_EXPERIMENT_STATE_C4 = 4;

class PassThroughExperimentHistory;
class PassThroughExperimentState;
class PassThroughExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	double existing_score;
	double existing_signal;

	int num_explores;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	int total_count;
	double total_sum_scores;

	double sum_scores;
	double sum_signals;

	PassThroughExperiment(Scope* scope_context,
						  AbstractNode* node_context,
						  bool is_branch,
						  SolutionWrapper* wrapper);
	~PassThroughExperiment();

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

	void measure_existing_check_activate(SolutionWrapper* wrapper);
	void measure_existing_backprop(double target_val,
								   SolutionWrapper* wrapper);

	void explore_check_activate(SolutionWrapper* wrapper);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper);
	void explore_set_action(int action,
							PassThroughExperimentState* experiment_state);
	void explore_exit_step(SolutionWrapper* wrapper,
						   PassThroughExperimentState* experiment_state);
	void explore_backprop(double target_val,
						  SolutionWrapper* wrapper);

	void clean();
	void add(SolutionWrapper* wrapper);
	double calc_new_score();
};

class PassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	PassThroughExperimentHistory(PassThroughExperiment* experiment);
};

class PassThroughExperimentState : public AbstractExperimentState {
public:
	int step_index;

	PassThroughExperimentState(PassThroughExperiment* experiment);
};

#endif /* PASS_THROUGH_EXPERIMENT_H */