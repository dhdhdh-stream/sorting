#include "experiment.h"

#include "constants.h"
#include "world_state.h"

using namespace std;

Experiment::Experiment(WorldState* parent,
					   bool is_obs,
					   int obs_index,
					   bool obs_is_greater,
					   Action* action,
					   vector<WorldState*> experiment_states) {
	this->type = EXPERIMENT_TYPE_EXPERIMENT;

	this->parent = parent;
	this->is_obs = is_obs;
	this->obs_index = obs_index;
	this->obs_is_greater = obs_is_greater;
	this->action = action;

	this->experiment_states = experiment_states;

	this->average_remaining_experiments_from_start = 1.0;

	this->misguess_histories.reserve(MEASURE_NUM_SAMPLES);

	this->state = EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;

	this->new_misguess = 0.0;
}

Experiment::~Experiment() {
	for (int s_index = 0; s_index < (int)this->experiment_states.size(); s_index++) {
		delete this->experiment_states[s_index];
	}
}
