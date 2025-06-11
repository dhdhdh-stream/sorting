#ifndef PASS_THROUGH_EXPERIMENT_H
#define PASS_THROUGH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Problem;
class Scope;
class Solution;

const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
/**
 * - have to remeasure
 *   - cannot rely on score gathered from other experiments' measure existing
 *     - score will be biased as based on path
 * 
 * - also fails on BranchNodes
 */
const int PASS_THROUGH_EXPERIMENT_STATE_INITIAL = 1;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST = 2;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND = 3;

class PassThroughExperimentHistory;
class PassThroughExperimentState;
class PassThroughExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int explore_iter;

	double existing_average_score;

	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;
	bool is_init;

	double sum_score;

	PassThroughExperiment(Scope* scope_context,
						  AbstractNode* node_context,
						  bool is_branch);
	void decrement(AbstractNode* experiment_node);

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

	void measure_existing_backprop(double target_val,
								   SolutionWrapper* wrapper);

	void explore_check_activate(SolutionWrapper* wrapper);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper,
					  PassThroughExperimentState* experiment_state);
	void explore_set_action(int action,
							PassThroughExperimentState* experiment_state);
	void explore_exit_step(SolutionWrapper* wrapper,
						   PassThroughExperimentState* experiment_state);
	void explore_backprop(double target_val,
						  SolutionWrapper* wrapper);

	void clean();
	void add(SolutionWrapper* wrapper);
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