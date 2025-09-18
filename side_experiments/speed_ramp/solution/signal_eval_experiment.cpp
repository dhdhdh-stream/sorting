#include "signal_eval_experiment.h"

#include <iostream>

#include "default_signal.h"
#include "globals.h"
#include "scope.h"
#include "signal.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

SignalEvalExperiment::SignalEvalExperiment() {
	this->default_signal = NULL;

	this->state = SIGNAL_EVAL_EXPERIMENT_STATE_GATHER;
}

SignalEvalExperiment::~SignalEvalExperiment() {
	for (int s_index = 0; s_index < (int)this->previous_signals.size(); s_index++) {
		delete this->previous_signals[s_index];
	}

	for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
		delete this->signals[s_index];
	}

	if (this->default_signal != NULL) {
		delete this->default_signal;
	}
}

void SignalEvalExperiment::add(SolutionWrapper* wrapper) {
	this->scope_context->signal_pre_actions = this->pre_actions;
	this->scope_context->signal_post_actions = this->post_actions;
	for (int s_index = 0; s_index < (int)this->scope_context->signals.size(); s_index++) {
		delete this->scope_context->signals[s_index];
	}
	this->scope_context->signals = this->signals;
	this->signals.clear();
	if (this->scope_context->default_signal != NULL) {
		delete this->scope_context->default_signal;
	}
	this->scope_context->default_signal = this->default_signal;
	this->default_signal = NULL;

	wrapper->solution->timestamp++;
}

SignalEvalExperimentHistory::SignalEvalExperimentHistory() {
	/**
	 * - high probability of signal experiment
	 *   - but existing signals still impactful as layers increase
	 */
	// uniform_int_distribution<int> experiment_distribution(0, 1);
	uniform_int_distribution<int> experiment_distribution(0, 3);
	if (experiment_distribution(generator) == 0) {
		this->is_on = true;
	} else {
		this->is_on = false;
	}
}
