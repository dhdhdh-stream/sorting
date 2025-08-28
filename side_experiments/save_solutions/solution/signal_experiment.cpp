#include "signal_experiment.h"

#include "globals.h"
#include "helpers.h"
#include "problem.h"
#include "scope.h"
#include "signal_instance.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

SignalExperiment::SignalExperiment(int scope_context_id,
								   SolutionWrapper* wrapper) {
	this->scope_context_id = scope_context_id;

	this->new_scope = NULL;
	this->miss_average_guess = 0.0;

	set_actions(wrapper);

	this->state = SIGNAL_EXPERIMENT_STATE_FIND_SAFE;
	this->state_iter = 0;
	this->solution_index = 0;
}

SignalExperiment::~SignalExperiment() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}

	for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
		delete this->signals[s_index];
	}
}

SignalExperimentInstanceHistory::SignalExperimentInstanceHistory() {
	this->signal_needed_from = NULL;
}

SignalExperimentState::SignalExperimentState(SignalExperiment* experiment) {
	this->experiment = experiment;
}
