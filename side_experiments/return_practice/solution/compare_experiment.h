#ifndef COMPARE_EXPERIMET_H
#define COMPARE_EXPERIMET_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class ExperimentRun;
class Network;
class Wrapper;

const int COMPARE_EXPERIMENT_MEASURE_EXISTING = 0;
const int COMPARE_EXPERIMENT_MEASURE_NEW = 1;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_ITERS = 20;
#else
const int MEASURE_NUM_ITERS = 4000;
#endif /* MDEBUG */

class CompareExperiment : public AbstractExperiment {
public:
	AbstractNode* node_context;
	bool is_branch;
	AbstractNode* exit_next_node;

	int state;
	int state_iter;

	Network* original_network;
	Network* branch_network;

	std::vector<int> actions;

	double existing_average_score;

	double sum_scores;

	double predicted_existing_average;
	double predicted_new_average;

	~CompareExperiment();

	void experiment_activate(ExperimentRun* run);
	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);
	void backprop(double target_val,
				  ExperimentRun* run,
				  Wrapper* wrapper);

	void measure_existing_experiment_activate(ExperimentRun* run);
	void measure_existing_backprop(double target_val,
								   ExperimentRun* run,
								   Wrapper* wrapper);

	void measure_new_experiment_activate(ExperimentRun* run);
	void measure_new_experiment_step(int& action,
									 bool& is_next,
									 ExperimentRun* run);
	void measure_new_backprop(double target_val,
							  ExperimentRun* run,
							  Wrapper* wrapper);
};

class CompareExperimentHistory {
public:
	CompareExperiment* experiment;

	bool hit_branch;

	CompareExperimentHistory(CompareExperiment* experiment);
};

class CompareExperimentState : public AbstractExperimentState {
public:
	int step_index;

	CompareExperimentState(CompareExperiment* experiment);
};

#endif /* COMPARE_EXPERIMET_H */