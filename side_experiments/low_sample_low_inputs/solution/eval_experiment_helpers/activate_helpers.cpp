#include "eval_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void EvalExperiment::experiment_check_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		switch (this->state) {
		case EVAL_EXPERIMENT_STATE_RAMP:
		case EVAL_EXPERIMENT_STATE_MEASURE:
			ramp_check_activate(wrapper);
			break;
		}
	}
}

void EvalExperiment::experiment_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 bool& fetch_action,
									 SolutionWrapper* wrapper) {
	switch (this->state) {
	case EVAL_EXPERIMENT_STATE_RAMP:
	case EVAL_EXPERIMENT_STATE_MEASURE:
		ramp_step(obs,
				  action,
				  is_next,
				  fetch_action,
				  wrapper);
		break;
	}
}

void EvalExperiment::set_action(int action,
								SolutionWrapper* wrapper) {
	// do nothing
}

void EvalExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	switch (this->state) {
	case EVAL_EXPERIMENT_STATE_RAMP:
	case EVAL_EXPERIMENT_STATE_MEASURE:
		ramp_exit_step(wrapper);
		break;
	}
}

void EvalExperiment::backprop(double target_val,
							  EvalExperimentHistory* history,
							  SolutionWrapper* wrapper,
							  set<Scope*>& updated_scopes) {
	switch (this->state) {
	case EVAL_EXPERIMENT_STATE_RAMP:
	case EVAL_EXPERIMENT_STATE_MEASURE:
		ramp_backprop(target_val,
					  history,
					  wrapper,
					  updated_scopes);
		break;
	}
}
