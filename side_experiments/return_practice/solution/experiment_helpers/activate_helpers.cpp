#include "experiment.h"

#include <iostream>
#include <limits>

#include "action_node.h"
#include "branch_network.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment_run.h"
#include "solution.h"
#include "start_node.h"
#include "utilities.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

void Experiment::experiment_activate(ExperimentRun* run) {
	switch (this->state) {
	case EXPERIMENT_STATE_GATHER:
		gather_activate(run);
		break;
	case EXPERIMENT_STATE_RAMP:
	case EXPERIMENT_STATE_MEASURE:
		ramp_activate(run);
		break;
	}
}

void Experiment::experiment_step(int& action,
								 bool& is_next,
								 ExperimentRun* run) {
	ramp_step(action,
			  is_next,
			  run);
}

void Experiment::experiment_exit(ExperimentRun* run) {
	gather_exit(run);
}

void Experiment::backprop(double target_val,
						  ExperimentHistory* history,
						  Wrapper* wrapper) {
	switch (this->state) {
	case EXPERIMENT_STATE_GATHER:
		gather_backprop(target_val,
						history,
						wrapper);
		break;
	case EXPERIMENT_STATE_RAMP:
	case EXPERIMENT_STATE_MEASURE:
		ramp_backprop(target_val,
					  history,
					  wrapper);
		break;
	}
}
