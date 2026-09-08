#include "predict_experiment.h"

#include "constants.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void PredictExperiment::experiment_check_activate(vector<double>& obs,
												  SolutionWrapper* wrapper) {
	map<PredictExperiment*, PredictExperimentHistory*>::iterator it =
		wrapper->predict_experiment_histories.find(this);
	if (it == wrapper->predict_experiment_histories.end()) {
		it = wrapper->predict_experiment_histories.insert({this, new PredictExperimentHistory(this)}).first;
	}

	switch (this->state) {
	case EXPERIMENT_STATE_GATHER_EXISTING:
		gather_existing_check_activate(obs,
									   it->second,
									   wrapper);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_check_activate(obs,
							   it->second,
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
	case EXPERIMENT_STATE_MEASURE:
		measure_step(obs,
					 action,
					 is_next,
					 wrapper);
		break;
	}
}

void PredictExperiment::set_action(int action,
								   SolutionWrapper* wrapper) {
	// not reachable
}

void PredictExperiment::experiment_step_callback(vector<double>& obs,
												 SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPERIMENT_STATE_MEASURE:
		measure_callback(obs,
						 wrapper);
		break;
	}
}

void PredictExperiment::experiment_exit_step(vector<double>& obs,
											 SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPERIMENT_STATE_MEASURE:
		measure_exit_step(obs,
						  wrapper);
		break;
	}
}

void PredictExperiment::backprop(double target_val,
								 PredictExperimentHistory* history,
								 SolutionWrapper* wrapper,
								 bool& is_add) {
	switch (this->state) {
	case EXPERIMENT_STATE_GATHER_EXISTING:
		gather_existing_backprop(target_val,
								 history,
								 wrapper);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 history,
						 wrapper,
						 is_add);
		break;
	}
}
