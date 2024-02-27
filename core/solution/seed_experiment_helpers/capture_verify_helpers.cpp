#if defined(MDEBUG) && MDEBUG

#include "seed_experiment.h"

#include "constants.h"

using namespace std;

void SeedExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		this->result = EXPERIMENT_RESULT_SUCCESS;
	}
}

#endif /* MDEBUG */