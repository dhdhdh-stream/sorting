#include "signal_experiment.h"

#include "constants.h"
#include "globals.h"
#include "signal.h"

using namespace std;

SignalExperiment::SignalExperiment(Scope* scope_context) {
	this->scope_context = scope_context;

	set_actions();

	this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C1;
	this->state_iter = 0;
}

SignalExperiment::~SignalExperiment() {
	for (int s_index = 0; s_index < (int)this->adjusted_previous_signals.size(); s_index++) {
		delete this->adjusted_previous_signals[s_index];
	}
}

SignalExperimentHistory::SignalExperimentHistory(SignalExperiment* experiment) {
	switch (experiment->state) {
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C1:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C2:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C3:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C4:
		{
			uniform_int_distribution<int> on_distribution(0, 99);
			if (on_distribution(generator) == 0) {
				this->is_on = true;
			} else {
				this->is_on = false;
			}
		}
		break;
	case SIGNAL_EXPERIMENT_STATE_RAMP:
		{
			uniform_int_distribution<int> on_distribution(0, EXPERIMENT_NUM_GEARS-1);
			if (experiment->curr_ramp >= on_distribution(generator)) {
				this->is_on = true;
			} else {
				this->is_on = false;
			}
		}
		break;
	}
}

SignalExperimentState::SignalExperimentState(SignalExperiment* experiment) {
	this->experiment = experiment;
}
