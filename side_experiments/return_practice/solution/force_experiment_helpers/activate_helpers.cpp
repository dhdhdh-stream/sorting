#include "force_experiment.h"

#include "experiment_run.h"

using namespace std;

void ForceExperiment::experiment_activate(ExperimentRun* run) {
	map<ForceExperiment*, ForceExperimentHistory*>::iterator it =
		run->force_experiment_histories.find(this);
	if (it == run->force_experiment_histories.end()) {
		run->force_experiment_histories[this] = new ForceExperimentHistory(this);
	}

	switch (this->state) {
	case FORCE_EXPERIMENT_STATE_EXPLORE:
		explore_experiment_activate(run);
		break;
	case FORCE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_experiment_activate(run);
		break;
	}
}

void ForceExperiment::experiment_step(int& action,
									  bool& is_next,
									  ExperimentRun* run) {
	switch (this->state) {
	case FORCE_EXPERIMENT_STATE_EXPLORE:
		explore_experiment_step(action,
								is_next,
								run);
		break;
	case FORCE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_experiment_step(action,
								  is_next,
								  run);
		break;
	}
}

void ForceExperiment::backprop(double target_val,
							   ExperimentRun* run,
							   ForceExperimentHistory* history,
							   Wrapper* wrapper) {
	switch (this->state) {
	case FORCE_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run,
						 history,
						 wrapper);
		break;
	case FORCE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run,
						   history,
						   wrapper);
		break;
	}
}
