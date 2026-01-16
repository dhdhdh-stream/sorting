#include "eval_experiment.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"

using namespace std;

EvalExperiment::EvalExperiment() {
	this->type = EXPERIMENT_TYPE_EVAL;

	this->new_network = NULL;
	this->new_scope = NULL;

	this->existing_sum_scores = 0.0;
	this->existing_count = 0;
	this->new_sum_scores = 0.0;
	this->new_count = 0;

	this->curr_ramp = 0;

	this->state = EVAL_EXPERIMENT_STATE_INIT;
	this->state_iter = 0;
	this->num_fail = 0;
}

EvalExperiment::~EvalExperiment() {
	if (this->new_network != NULL) {
		delete this->new_network;
	}

	if (this->new_scope != NULL) {
		delete this->new_scope;
	}
}

EvalExperimentHistory::EvalExperimentHistory(EvalExperiment* experiment) {
	uniform_int_distribution<int> on_distribution(0, EXPERIMENT_NUM_GEARS-1);
	if (experiment->curr_ramp >= on_distribution(generator)) {
		this->is_on = true;
	} else {
		this->is_on = false;
	}
}

EvalExperimentState::EvalExperimentState(EvalExperiment* experiment) {
	this->experiment = experiment;
}
