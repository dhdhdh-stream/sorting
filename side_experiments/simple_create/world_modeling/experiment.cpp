#include "experiment.h"

#include "constants.h"
#include "hidden_state.h"

using namespace std;

Experiment::Experiment(HiddenState* parent,
					   vector<HiddenState*> experiment_states) {
	this->type = EXPERIMENT_TYPE_EXPERIMENT;

	this->parent = parent;
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
