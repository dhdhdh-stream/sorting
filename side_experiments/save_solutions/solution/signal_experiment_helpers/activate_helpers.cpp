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

void SignalExperiment::check_activate(AbstractNode* experiment_node,
									  bool is_branch,
									  SolutionWrapper* wrapper) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		explore_check_activate(experiment_node,
							   is_branch,
							   wrapper);
		break;
	}
}

void SignalExperiment::experiment_step(std::vector<double>& obs,
									   int& action,
									   bool& is_next,
									   bool& fetch_action,
									   SolutionWrapper* wrapper) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		explore_experiment_step(obs,
								action,
								is_next,
								fetch_action,
								wrapper);
		break;
	}
}

void SignalExperiment::set_action(int action,
								  SolutionWrapper* wrapper) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		explore_set_action(action,
						   wrapper);
		break;
	}
}

void SignalExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		explore_experiment_exit_step(wrapper);
		break;
	}
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
