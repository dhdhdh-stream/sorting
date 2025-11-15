#include "experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void Experiment::check_activate(AbstractNode* experiment_node,
								bool is_branch,
								SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
		history->is_hit = true;

		switch (this->state) {
		case EXPERIMENT_STATE_TRAIN_NEW:
			train_new_check_activate(wrapper);
			break;
		case EXPERIMENT_STATE_MEASURE:
			measure_check_activate(wrapper);
			break;
		#if defined(MDEBUG) && MDEBUG
		case EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_check_activate(wrapper);
			break;
		#endif /* MDEBUG */
		}
	}
}

void Experiment::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 bool& fetch_action,
								 SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_step(obs,
					   action,
					   is_next,
					   wrapper);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_step(obs,
					 action,
					 is_next,
					 wrapper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_step(obs,
							action,
							is_next,
							wrapper);
		break;
	#endif /* MDEBUG */
	}
}

void Experiment::set_action(int action,
							SolutionWrapper* wrapper) {
	// do nothing
}

void Experiment::experiment_exit_step(SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];
	switch (this->state) {
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(wrapper,
							experiment_state);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_exit_step(wrapper,
						  experiment_state);
		break;
	#if defined(MDEBUG) && MDEBUG
	case EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_exit_step(wrapper,
								 experiment_state);
		break;
	#endif /* MDEBUG */
	}
}

void Experiment::backprop(double target_val,
						  SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   wrapper);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 wrapper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop(wrapper);
		break;
	#endif /* MDEBUG */
	}
}
