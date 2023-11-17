/**
 * - don't worry about misguess
 *   - unlikely to learn enough information for new path
 * 
 * - run MEASURE_EXISTING_SCORE once, then repeatedly run EXPLORE and MEASURE_NEW_SCORE
 */

#ifndef OUTER_EXPERIMENT_H
#define OUTER_EXPERIMENT_H

#include <vector>

#include "problem.h"
#include "run_helper.h"

class ActionNode;
class ScopeNode;
class Sequence;

const int OUTER_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE = 0;

const int OUTER_EXPERIMENT_STATE_EXPLORE = 1;
const int OUTER_EXPERIMENT_STATE_MEASURE_NEW_SCORE = 2;
const int OUTER_EXPERIMENT_STATE_VERIFY_EXISTING_SCORE = 3;
const int OUTER_EXPERIMENT_STATE_VERIFY_NEW_SCORE = 4;

const int OUTER_EXPERIMENT_STATE_SUCCESS = 5;

class OuterExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_variance;

	double curr_score;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<Sequence*> curr_sequences;
	std::vector<ScopeNode*> curr_root_scope_nodes;

	double best_score;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<Sequence*> best_sequences;
	std::vector<ScopeNode*> best_root_scope_nodes;

	std::vector<double> target_val_histories;

	OuterExperiment();
	~OuterExperiment();

	void activate(Problem& problem,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void measure_existing_score_activate(Problem& problem,
										 RunHelper& run_helper);
	void measure_existing_score_backprop(double target_val,
										 RunHelper& run_helper);

	void explore_initial_activate(Problem& problem,
								  RunHelper& run_helper);
	void explore_activate(Problem& problem,
						  RunHelper& run_helper);
	void explore_backprop(double target_val);

	void measure_new_score_activate(Problem& problem,
									RunHelper& run_helper);
	void measure_new_score_backprop(double target_val);

	void verify_existing_score_activate(Problem& problem,
										RunHelper& run_helper);
	void verify_existing_score_backprop(double target_val,
										RunHelper& run_helper);

	void verify_new_score_activate(Problem& problem,
								   RunHelper& run_helper);
	void verify_new_score_backprop(double target_val);
};

#endif /* OUTER_EXPERIMENT_H */