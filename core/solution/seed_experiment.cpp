#include "seed_experiment.h"

using namespace std;



SeedExperiment::~SeedExperiment() {


	if (this->state > SEED_EXPERIMENT_STATE_EXPLORE) {
		// TODO: clean seed path from local scope

	}

}
