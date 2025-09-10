#include "eval_experiment.h"

#include "globals.h"

using namespace std;

EvalExperiment::EvalExperiment() {
	this->state = EVAL_EXPERIMENT_STATE_INITIAL;
	this->state_iter = 0;
}



EvalExperimentHistory::EvalExperimentHistory(EvalExperiment* experiment) {
	switch (experiment->state) {
	case EVAL_EXPERIMENT_STATE_INITIAL:
		{
			uniform_int_distribution<int> on_distribution(0, 99);
			if (on_distribution(generator) == 0) {
				this->is_on = true;
			} else {
				this->is_on = false;
			}
		}
		break;
	case EVAL_EXPERIMENT_STATE_EVAL:
		{
			uniform_int_distribution<int> on_distribution(0, EVAL_EXPERIMENT_NUM_GEARS-1);
			if (experiment->curr_ramp >= on_distribution(generator)) {
				this->is_on = true;
			} else {
				this->is_on = false;
			}
		}
		break;
	}
}

EvalExperimentState::EvalExperimentState(EvalExperiment* experiment) {
	this->experiment = experiment;
}
