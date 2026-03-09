#include "eval_experiment.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

EvalExperiment::EvalExperiment(SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_EVAL;

	this->new_scope = NULL;

	this->num_original = 0;
	this->num_branch = 0;

	// this->state = EVAL_EXPERIMENT_STATE_REFINE;
	// this->state_iter = 0;

	this->starting_experiment_iter = wrapper->experiment_iter;

	this->existing_sum_scores = 0.0;
	this->existing_count = 0;

	this->state = EVAL_EXPERIMENT_STATE_MEASURE;
	this->state_iter = 0;
}

EvalExperiment::~EvalExperiment() {
	for (int n_index = 0; n_index < (int)this->new_networks.size(); n_index++) {
		delete this->new_networks[n_index];
	}

	if (this->new_scope != NULL) {
		delete this->new_scope;
	}
}

EvalExperimentHistory::EvalExperimentHistory(EvalExperiment* experiment) {
	switch (experiment->state) {
	case EVAL_EXPERIMENT_STATE_REFINE:
	case EVAL_EXPERIMENT_STATE_MEASURE:
		{
			uniform_int_distribution<int> on_distribution(0, 39);
			if (on_distribution(generator) == 0) {
				this->is_on = true;
			} else {
				this->is_on = false;
			}
		}
		break;
	case EVAL_EXPERIMENT_STATE_INIT:
	case EVAL_EXPERIMENT_STATE_RAMP:
		{
			uniform_int_distribution<int> on_distribution(0, EXPERIMENT_NUM_GEARS);
			if (experiment->curr_ramp >= on_distribution(generator)) {
				this->is_on = true;
			} else {
				this->is_on = false;
			}
		}
		break;
	}

	this->hit_branch = false;
}

EvalExperimentState::EvalExperimentState(EvalExperiment* experiment) {
	this->experiment = experiment;
}
