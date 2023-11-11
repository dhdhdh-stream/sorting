#include "outer_experiment.h"

using namespace std;

OuterExperiment::OuterExperiment() {
	this->state = OUTER_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE;
	this->state_iter = 0;

	this->curr_score = 0.0;

	this->best_score = 0.0;
}

OuterExperiment::~OuterExperiment() {
	// do nothing
}
