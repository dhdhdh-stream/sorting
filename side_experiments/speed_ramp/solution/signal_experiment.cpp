#include "signal_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "signal.h"
#include "solution_wrapper.h"

using namespace std;

SignalExperiment::SignalExperiment(Scope* scope_context) {
	this->type = EXPERIMENT_TYPE_SIGNAL;

	this->scope_context = scope_context;

	this->sum_num_following_explores = 0;

	set_actions();

	this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C1;
	this->state_iter = 0;
}

SignalExperiment::~SignalExperiment() {
	for (int s_index = 0; s_index < (int)this->adjusted_previous_pre_signals.size(); s_index++) {
		delete this->adjusted_previous_pre_signals[s_index];
	}
	for (int s_index = 0; s_index < (int)this->adjusted_previous_post_signals.size(); s_index++) {
		delete this->adjusted_previous_post_signals[s_index];
	}
}

SignalExperimentHistory::SignalExperimentHistory(
		SignalExperiment* experiment,
		SolutionWrapper* wrapper) {
	switch (experiment->state) {
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C1:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C2:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C3:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C4:
		this->is_on = false;
		if (wrapper->should_explore
				&& wrapper->curr_explore == NULL
				&& experiment->last_num_following_explores.size() > 0) {
			double average_num_follow = (double)experiment->sum_num_following_explores
				/ (double)experiment->last_num_following_explores.size();
			uniform_real_distribution<double> distribution(0.0, 1.0);
			double rand_val = distribution(generator);
			if (rand_val <= 1.0 / (1.0 + average_num_follow)) {
				wrapper->curr_explore = experiment;
				this->is_on = true;
			}
		}
		break;
	case SIGNAL_EXPERIMENT_STATE_RAMP:
	case SIGNAL_EXPERIMENT_STATE_WRAPUP:
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
