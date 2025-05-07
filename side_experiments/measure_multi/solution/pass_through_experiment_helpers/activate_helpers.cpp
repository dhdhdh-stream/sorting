#include "pass_through_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 5;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 10;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 40;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 400;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 4000;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 100;
#endif /* MDEBUG */

void PassThroughExperiment::activate(AbstractNode* experiment_node,
									 bool is_branch,
									 AbstractNode*& curr_node,
									 Problem* problem,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	PassThroughExperimentHistory* history;
	if (this->is_branch == is_branch) {
		map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
			= run_helper.experiment_histories.find(this);
		if (it == run_helper.experiment_histories.end()) {
			history = new PassThroughExperimentHistory(this);
			run_helper.experiment_histories[this] = history;
		} else {
			history = (PassThroughExperimentHistory*)it->second;
		}

		switch (this->state) {
		case PASS_THROUGH_EXPERIMENT_STATE_INITIAL:
		case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
		case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
			explore_activate(curr_node,
							 problem,
							 run_helper);
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_MULTI_MEASURE:
			multi_measure_activate(curr_node,
								   problem,
								   run_helper,
								   history);
			break;
		}
	}
}

void PassThroughExperiment::backprop(double target_val,
									 RunHelper& run_helper) {
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_INITIAL:
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
		explore_backprop(target_val,
						 run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MULTI_MEASURE:
		multi_measure_backprop(target_val,
							   run_helper);
		break;
	}
}
