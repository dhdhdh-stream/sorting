#include "experiment.h"

#include "constants.h"
#include "hidden_state.h"

using namespace std;

Experiment::Experiment(HiddenState* parent,
					   int starting_action,
					   vector<HiddenState*> experiment_states) {
	this->parent = parent;
	this->starting_action = starting_action;
	this->experiment_states = experiment_states;

	this->average_remaining_experiments_from_start = 1.0;

	this->misguess_histories.reserve(MEASURE_NUM_SAMPLES);

	this->state = EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->new_misguess = 0.0;
}

Experiment::~Experiment() {
	for (int s_index = 0; s_index < (int)this->experiment_states.size(); s_index++) {
		delete this->experiment_states[s_index];
	}
}
