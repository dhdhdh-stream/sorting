#include "signal_experiment.h"

#include "solution_wrapper.h"

using namespace std;

void SignalExperiment::check_activate(AbstractNode* experiment_node,
									  bool is_branch,
									  SolutionWrapper* wrapper) {
	/**
	 * - unused
	 */
}

void SignalExperiment::experiment_step(std::vector<double>& obs,
									   int& action,
									   bool& is_next,
									   bool& fetch_action,
									   SolutionWrapper* wrapper) {
	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
	/**
	 * - simply delete state and recreate if needed
	 */
}

void SignalExperiment::set_action(int action,
								  SolutionWrapper* wrapper) {
	SignalExperimentState* state = (SignalExperimentState*)wrapper->experiment_context.back();
	if (state->is_pre) {
		this->pre_action_initialized[state->index] = true;
		this->pre_actions[state->index] = action;
	} else {
		this->post_action_initialized[state->index] = true;
		this->post_actions[state->index] = action;
	}
}

void SignalExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	/**
	 * - unused
	 */
}

void SignalExperiment::backprop(double target_val,
								SignalExperimentHistory* history,
								SolutionWrapper* wrapper) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C1:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C2:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C3:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C4:
		initial_backprop(target_val,
						 history,
						 wrapper);
		break;
	case SIGNAL_EXPERIMENT_STATE_RAMP:
		ramp_backprop(target_val,
					  history,
					  wrapper);
		break;
	case SIGNAL_EXPERIMENT_STATE_WRAPUP:
		wrapup_backprop();
		break;
	}
}
