#include "pass_through_experiment.h"

#include "solution_wrapper.h"

using namespace std;

void PassThroughExperiment::check_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)wrapper->experiment_history;
		history->is_hit = true;

		switch (this->state) {
		case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
			measure_existing_check_activate(wrapper);
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_C1:
		case PASS_THROUGH_EXPERIMENT_STATE_C2:
		case PASS_THROUGH_EXPERIMENT_STATE_C3:
		case PASS_THROUGH_EXPERIMENT_STATE_C4:
			explore_check_activate(wrapper);
			break;
		}
	}
}

void PassThroughExperiment::experiment_step(
		vector<double>& obs,
		int& action,
		bool& is_next,
		bool& fetch_action,
		SolutionWrapper* wrapper) {
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_C1:
	case PASS_THROUGH_EXPERIMENT_STATE_C2:
	case PASS_THROUGH_EXPERIMENT_STATE_C3:
	case PASS_THROUGH_EXPERIMENT_STATE_C4:
		explore_step(obs,
					 action,
					 is_next,
					 fetch_action,
					 wrapper);
		break;
	}
}

void PassThroughExperiment::set_action(int action,
									   SolutionWrapper* wrapper) {
	PassThroughExperimentState* experiment_state = (PassThroughExperimentState*)wrapper->experiment_context.back();
	explore_set_action(action,
					   experiment_state);
}

void PassThroughExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	PassThroughExperimentState* experiment_state = (PassThroughExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_C1:
	case PASS_THROUGH_EXPERIMENT_STATE_C2:
	case PASS_THROUGH_EXPERIMENT_STATE_C3:
	case PASS_THROUGH_EXPERIMENT_STATE_C4:
		explore_exit_step(wrapper,
						  experiment_state);
		break;
	}
}

void PassThroughExperiment::backprop(double target_val,
									 SolutionWrapper* wrapper) {
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  wrapper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_C1:
	case PASS_THROUGH_EXPERIMENT_STATE_C2:
	case PASS_THROUGH_EXPERIMENT_STATE_C3:
	case PASS_THROUGH_EXPERIMENT_STATE_C4:
		explore_backprop(target_val,
						 wrapper);
		break;
	}
}
