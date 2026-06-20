#include "compare_experiment.h"

using namespace std;

void CompareExperiment::experiment_activate(ExperimentRun* run) {
	switch (this->state) {
	case COMPARE_EXPERIMENT_MEASURE_EXISTING:
		measure_existing_experiment_activate(run);
		break;
	case COMPARE_EXPERIMENT_MEASURE_NEW:
		measure_new_experiment_activate(run);
		break;
	}
}

void CompareExperiment::experiment_step(int& action,
										bool& is_next,
										ExperimentRun* run) {
	measure_new_experiment_step(action,
								is_next,
								run);
}

void CompareExperiment::backprop(double target_val,
								 ExperimentRun* run,
								 Wrapper* wrapper) {
	switch (this->state) {
	case COMPARE_EXPERIMENT_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run,
								  wrapper);
		break;
	case COMPARE_EXPERIMENT_MEASURE_NEW:
		measure_new_backprop(target_val,
							 run,
							 wrapper);
		break;
	}
}
