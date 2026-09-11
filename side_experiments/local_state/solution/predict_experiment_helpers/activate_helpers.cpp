#include "predict_experiment.h"

#include <iostream>

#include "constants.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void PredictExperiment::experiment_check_activate(vector<double>& obs,
												  SolutionWrapper* wrapper) {
	map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it =
		wrapper->experiment_histories[this->diversity_index].find(this);
	if (it == wrapper->experiment_histories[this->diversity_index].end()) {
		it = wrapper->experiment_histories[this->diversity_index].insert({this, new PredictExperimentHistory(this)}).first;
	}
	PredictExperimentHistory* predict_experiment_history = (PredictExperimentHistory*)it->second;

	switch (this->state) {
	case PREDICT_EXPERIMENT_STATE_GATHER_EXISTING:
		gather_existing_check_activate(obs,
									   predict_experiment_history,
									   wrapper);
		break;
	case PREDICT_EXPERIMENT_STATE_MEASURE:
		measure_check_activate(obs,
							   predict_experiment_history,
							   wrapper);
		break;
	}
}

void PredictExperiment::experiment_step(vector<double>& obs,
										int& action,
										bool& is_next,
										bool& fetch_action,
										SolutionWrapper* wrapper) {
	switch (this->state) {
	case PREDICT_EXPERIMENT_STATE_MEASURE:
		measure_step(obs,
					 action,
					 is_next,
					 wrapper);
		break;
	}
}

void PredictExperiment::set_action(int action,
								   SolutionWrapper* wrapper) {
	// unreachable
}

void PredictExperiment::experiment_step_callback(vector<double>& obs,
												 SolutionWrapper* wrapper) {
	switch (this->state) {
	case PREDICT_EXPERIMENT_STATE_MEASURE:
		measure_callback(obs,
						 wrapper);
		break;
	}
}

void PredictExperiment::experiment_exit_step(vector<double>& obs,
											 SolutionWrapper* wrapper) {
	switch (this->state) {
	case PREDICT_EXPERIMENT_STATE_MEASURE:
		measure_exit_step(obs,
						  wrapper);
		break;
	}
}

void PredictExperiment::backprop(double target_val,
								 AbstractExperimentHistory* history,
								 SolutionWrapper* wrapper) {
	PredictExperimentHistory* predict_experiment_history = (PredictExperimentHistory*)history;

	switch (this->state) {
	case PREDICT_EXPERIMENT_STATE_GATHER_EXISTING:
		gather_existing_backprop(target_val,
								 predict_experiment_history,
								 wrapper);
		break;
	case PREDICT_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 predict_experiment_history,
						 wrapper);
		break;
	}
}
