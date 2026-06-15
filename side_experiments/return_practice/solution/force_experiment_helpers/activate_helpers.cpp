#include "force_experiment.h"

using namespace std;

void ForceExperiment::experiment_activate(ExperimentRun* run) {
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

void ForceExperiment::predict_activate(PredictRun* run) {
	// unreachable
}

void ForceExperiment::backprop(double target_val,
							   ExperimentRun* run,
							   Wrapper* wrapper) {
	switch (this->state) {
	case FORCE_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run,
						 wrapper);
		break;
	case FORCE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run,
						   wrapper);
		break;
	}
}
