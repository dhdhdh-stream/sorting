#include "signal_experiment.h"

#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

SignalExperiment::SignalExperiment() {
	this->state = SIGNAL_EXPERIMENT_STATE_MEASURE_EXISTING;
}

void SignalExperiment::add(SolutionWrapper* wrapper) {
	Scope* scope = wrapper->solution->scopes[0];
	scope->signal_pre_actions = this->pre_actions;
	scope->signal_post_actions = this->post_actions;
	scope->signals = this->signals;
	scope->miss_average_guess = this->miss_average_guess;
}
