#include "seed_experiment.h"

#include <iostream>

using namespace std;

void SeedExperiment::find_gather_backprop(SeedExperimentOverallHistory* history) {
	if (this->curr_gather != NULL) {
		this->state = SEED_EXPERIMENT_STATE_FIND_GATHER_SEED;
		this->sub_state_iter = 0;
	}
}
