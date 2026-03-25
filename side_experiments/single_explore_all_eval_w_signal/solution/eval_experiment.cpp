#include "eval_experiment.h"

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

EvalExperiment::EvalExperiment() {
	this->best_new_scope = NULL;

	this->curr_ramp = 0;
	this->measure_status = MEASURE_STATUS_N_A;

	this->state = EVAL_EXPERIMENT_STATE_RAMP;
	this->state_iter = 0;
}

EvalExperiment::~EvalExperiment() {
	if (this->new_network != NULL) {
		delete this->new_network;
	}

	if (this->best_new_scope != NULL) {
		delete this->best_new_scope;
	}
}

EvalExperimentHistory::EvalExperimentHistory(EvalExperiment* experiment) {
	this->experiment = experiment;

	uniform_int_distribution<int> on_distribution(0, EXPERIMENT_NUM_GEARS);
	if (experiment->curr_ramp >= on_distribution(generator)) {
		this->is_on = true;
	} else {
		this->is_on = false;
	}

	this->hit_branch = false;
}

EvalExperimentState::EvalExperimentState(EvalExperiment* experiment) {
	this->experiment = experiment;
}
