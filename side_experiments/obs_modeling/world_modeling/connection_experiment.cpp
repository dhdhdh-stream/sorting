#include "connection_experiment.h"

#include "constants.h"

using namespace std;

ConnectionExperiment::ConnectionExperiment(WorldState* parent,
										   bool is_obs,
										   int obs_index,
										   bool obs_is_greater,
										   Action* action,
										   WorldState* target) {
	this->type = EXPERIMENT_TYPE_CONNECTION;

	this->parent = parent;
	this->is_obs = is_obs;
	this->obs_index = obs_index;
	this->obs_is_greater = obs_is_greater;
	this->action = action;

	this->target = target;

	this->average_remaining_experiments_from_start = 1.0;

	this->misguess_histories.reserve(MEASURE_NUM_SAMPLES);

	this->state = CONNECTION_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;

	this->new_misguess = 0.0;
}
