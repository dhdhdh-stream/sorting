#ifndef OUTER_EXPERIMENT_H
#define OUTER_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "run_helper.h"

class ActionNode;
class Problem;
class ScopeNode;

const int OUTER_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int OUTER_EXPERIMENT_STATE_EXPLORE = 1;
const int OUTER_EXPERIMENT_STATE_MEASURE_NEW = 2;
const int OUTER_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 3;
const int OUTER_EXPERIMENT_STATE_VERIFY_1ST_NEW = 4;
const int OUTER_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 5;
const int OUTER_EXPERIMENT_STATE_VERIFY_2ND_NEW = 6;

class OuterExperimentOverallHistory;
class OuterExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_variance;

	double curr_score;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_existing_scopes;
	std::vector<ScopeNode*> curr_potential_scopes;

	double best_score;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_existing_scopes;
	std::vector<ScopeNode*> best_potential_scopes;

	std::vector<double> target_val_histories;

	OuterExperiment();
	~OuterExperiment();

	bool activate(Problem* problem,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper,
				  AbstractExperimentHistory* history);

	void measure_existing_activate(Problem* problem,
								   RunHelper& run_helper);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void explore_initial_activate(Problem* problem,
								  RunHelper& run_helper);
	void explore_activate(Problem* problem,
						  RunHelper& run_helper);
	void explore_backprop(double target_val);

	void measure_new_activate(Problem* problem,
							  RunHelper& run_helper);
	void measure_new_backprop(double target_val);

	void verify_existing_activate(Problem* problem,
								  RunHelper& run_helper);
	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_new_activate(Problem* problem,
							 RunHelper& run_helper);
	void verify_new_backprop(double target_val);

	void finalize();

	// unused
	bool activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  AbstractExperimentHistory*& history);
};

class OuterExperimentOverallHistory : public AbstractExperimentHistory {
public:
	OuterExperimentOverallHistory(OuterExperiment* experiment);
};

#endif /* OUTER_EXPERIMENT_H */