#include "pass_through_experiment.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void PassThroughExperiment::check_activate(AbstractNode* experiment_node,
										   bool is_branch,
										   SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		if (wrapper->experiment_history == NULL) {
			wrapper->experiment_history = new PassThroughExperimentHistory(this);
		}

		switch (this->state) {
		case PASS_THROUGH_EXPERIMENT_STATE_INITIAL:
		case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
		case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
			explore_check_activate(wrapper);
			break;
		}
	}
}

void PassThroughExperiment::experiment_step(vector<double>& obs,
											int& action,
											bool& is_next,
											SolutionWrapper* wrapper) {
	PassThroughExperimentState* experiment_state = (PassThroughExperimentState*)wrapper->experiment_context.back();
	explore_step(obs,
				 action,
				 is_next,
				 wrapper,
				 experiment_state);
}

void PassThroughExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	PassThroughExperimentState* experiment_state = (PassThroughExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];
	explore_exit_step(wrapper,
					  experiment_state);
}

void PassThroughExperiment::backprop(double target_val,
									 SolutionWrapper* wrapper) {
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  wrapper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_INITIAL:
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
		explore_backprop(target_val,
						 wrapper);
		break;
	}
}
