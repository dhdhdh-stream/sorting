#include "signal_experiment.h"

#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

bool SignalExperiment::check_signal(vector<double>& obs,
									int& action,
									bool& is_next,
									SolutionWrapper* wrapper) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_FIND_SAFE:
		return find_safe_check_signal(obs,
									  action,
									  is_next,
									  wrapper);
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		return explore_check_signal(obs,
									action,
									is_next,
									wrapper);
	}

	return false;
}

void SignalExperiment::backprop(double target_val,
								SolutionWrapper* wrapper) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_FIND_SAFE:
		find_safe_backprop(target_val,
						   wrapper);
		break;
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 wrapper);
		break;
	}
}
