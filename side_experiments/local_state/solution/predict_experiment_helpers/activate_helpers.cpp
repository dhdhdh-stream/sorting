#include "predict_experiment.h"

#include <iostream>

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

	it->second->state_histories.push_back(wrapper->states.back());

	it->second->signal_histories.push_back(0.0);
	wrapper->scope_histories.back()->experiment_callback_histories.push_back(it->second);
	wrapper->scope_histories.back()->experiment_callback_indexes.push_back(it->second->signal_histories.size()-1);
}

void PredictExperiment::experiment_step(vector<double>& obs,
										int& action,
										bool& is_next,
										bool& fetch_action,
										SolutionWrapper* wrapper) {
	// unreachable
}

void PredictExperiment::set_action(int action,
								   SolutionWrapper* wrapper) {
	// unreachable
}

void PredictExperiment::experiment_exit_step(vector<double>& obs,
											 SolutionWrapper* wrapper) {
	// unreachable
}

void PredictExperiment::experiment_step_callback(vector<double>& obs,
												 SolutionWrapper* wrapper) {
	// unreachable
}

void PredictExperiment::backprop(double target_val,
								 PredictExperimentHistory* history,
								 SolutionWrapper* wrapper,
								 bool& is_add) {
	for (int i_index = 0; i_index < (int)history->state_histories.size(); i_index++) {
		this->existing_state_histories.push_back(history->state_histories[i_index]);
		this->existing_signal_histories.push_back(history->signal_histories[i_index]);
		this->existing_target_val_histories.push_back(target_val);
	}

	this->state_iter++;
	if (this->state_iter >= EXPERIMENT_TRAIN_NUM_DATAPOINTS) {
		train_existing_helper();

		explore_helper();
		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->best_surprise >= 0.0) {
		#endif /* MDEBUG */
			train_new_helper(wrapper,
							 is_add);
		} else {
			delete this;
		}
	}
}
