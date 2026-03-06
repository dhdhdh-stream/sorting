#include "repetition_experiment.h"

#include "solution_wrapper.h"

using namespace std;

void RepetitionExperiment::check_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		SolutionWrapper* wrapper) {
	RepetitionExperimentHistory* history = (RepetitionExperimentHistory*)wrapper->experiment_history;
	history->is_hit = true;

	switch (this->state) {
	case REPETITION_EXPERIMENT_STATE_MEASURE_NEW:
		measure_new_check_activate(wrapper);
		break;
	}
}

void RepetitionExperiment::experiment_step(
		vector<double>& obs,
		int& action,
		bool& is_next,
		bool& fetch_action,
		SolutionWrapper* wrapper) {
	measure_new_step(obs,
					 action,
					 is_next,
					 wrapper);
}

void RepetitionExperiment::set_action(int action,
									  SolutionWrapper* wrapper) {
	// do nothing
}

void RepetitionExperiment::experiment_exit_step(
		SolutionWrapper* wrapper) {
	measure_new_exit_step(wrapper);
}

void RepetitionExperiment::backprop(double target_val,
									SolutionWrapper* wrapper) {
	switch (this->state) {
	case REPETITION_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  wrapper);
		break;
	case REPETITION_EXPERIMENT_STATE_MEASURE_NEW:
		measure_new_backprop(target_val,
							 wrapper);
		break;
	}
}
