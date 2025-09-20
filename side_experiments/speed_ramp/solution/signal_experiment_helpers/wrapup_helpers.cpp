#include "signal_experiment.h"

#include "constants.h"
#include "scope.h"

using namespace std;

void SignalExperiment::wrapup_backprop() {
	this->state_iter++;
	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
		this->state_iter = 0;

		this->curr_ramp--;
		if (this->curr_ramp < 0) {
			this->scope_context->signal_experiment = NULL;
			delete this;
		}
	}
}
