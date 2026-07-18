#include "multi_experiment.h"

#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void MultiExperiment::experiment_check_activate(vector<double>& obs,
												SolutionWrapper* wrapper) {
	MultiExperimentHistory* history;
	map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it =
		wrapper->experiment_histories.find(this);
	if (it == wrapper->experiment_histories.end()) {
		history = new MultiExperimentHistory(this);
		wrapper->experiment_histories[this] = history;
	} else {
		history = (MultiExperimentHistory*)it->second;
	}
	history->num_instances++;

	switch (this->state) {
	case MULTI_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_check_activate(obs,
									  history,
									  wrapper);
		break;
	case MULTI_EXPERIMENT_STATE_EXPLORE:
		explore_check_activate(obs,
							   history,
							   wrapper);
		break;
	case MULTI_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_check_activate(obs,
								 history,
								 wrapper);
		break;
	case MULTI_EXPERIMENT_STATE_MEASURE:
		measure_check_activate(obs,
							   history,
							   wrapper);
		break;
	}
}

void MultiExperiment::experiment_step(vector<double>& obs,
									  int& action,
									  bool& is_next,
									  bool& fetch_action,
									  SolutionWrapper* wrapper) {
	switch (this->state) {
	case MULTI_EXPERIMENT_STATE_EXPLORE:
		explore_step(obs,
					 action,
					 is_next,
					 fetch_action,
					 wrapper);
		break;
	case MULTI_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_step(obs,
					   action,
					   is_next,
					   wrapper);
		break;
	case MULTI_EXPERIMENT_STATE_MEASURE:
		measure_step(obs,
					 action,
					 is_next,
					 wrapper);
		break;
	}
}

void MultiExperiment::set_action(int action,
								 SolutionWrapper* wrapper) {
	explore_set_action(action,
					   wrapper);
}

void MultiExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	switch (this->state) {
	case MULTI_EXPERIMENT_STATE_EXPLORE:
		explore_exit_step(wrapper);
		break;
	case MULTI_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(wrapper);
		break;
	case MULTI_EXPERIMENT_STATE_MEASURE:
		measure_exit_step(wrapper);
		break;
	}
}

void MultiExperiment::backprop(double target_val,
							   AbstractExperimentHistory* history,
							   SolutionWrapper* wrapper) {
	MultiExperimentHistory* multi_experiment_history = (MultiExperimentHistory*)history;

	this->average_instances_per_hit = 0.99*this->average_instances_per_hit + 0.01*multi_experiment_history->num_instances;

	switch (this->state) {
	case MULTI_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								multi_experiment_history,
								wrapper);
		break;
	case MULTI_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 multi_experiment_history,
						 wrapper);
		break;
	case MULTI_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   multi_experiment_history,
						   wrapper);
		break;
	case MULTI_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 multi_experiment_history,
						 wrapper);
		break;
	}
}
