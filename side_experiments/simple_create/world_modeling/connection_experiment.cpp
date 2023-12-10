#include "connection_experiment.h"

#include "constants.h"

using namespace std;

ConnectionExperiment::ConnectionExperiment(HiddenState* parent,
										   HiddenState* target) {
	this->type = EXPERIMENT_TYPE_CONNECTION;

	this->parent = parent;
	this->target = target;

	this->average_remaining_experiments_from_start = 1.0;

	this->misguess_histories.reserve(MEASURE_NUM_SAMPLES);

	this->state = CONNECTION_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;

	this->new_misguess = 0.0;
}
