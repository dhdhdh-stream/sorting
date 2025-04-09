#include "eval_experiment.h"

using namespace std;

void EvalExperiment::activate(Problem* problem) {
	switch (this->state) {
	case EVAL_EXPERIMENT_TRAIN:
		train_activate(problem);
		break;
	case EVAL_EXPERIMENT_MEASURE:
		measure_activate(problem);
		break;
	}
}

void EvalExperiment::backprop(double target_val) {
	switch (this->state) {
	case EVAL_EXPERIMENT_TRAIN:
		train_backprop(target_val);
		break;
	case EVAL_EXPERIMENT_MEASURE:
		measure_backprop(target_val);
		break;
	}
}
