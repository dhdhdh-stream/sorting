#include "predict_experiment.h"

#include "constants.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void PredictExperiment::gather_existing_check_activate(
		vector<double>& obs,
		PredictExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXISTING
			|| wrapper->diversity_index == this->diversity_index) {
		history->state_histories.push_back(wrapper->states.back());

		history->signal_histories.push_back(0.0);
		wrapper->scope_histories.back()->experiment_callback_histories.push_back(history);
		wrapper->scope_histories.back()->experiment_callback_indexes.push_back(history->signal_histories.size()-1);
	}
}

void PredictExperiment::gather_existing_backprop(
		double target_val,
		PredictExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXISTING
			|| wrapper->diversity_index == this->diversity_index) {
		for (int i_index = 0; i_index < (int)history->state_histories.size(); i_index++) {
			this->existing_state_histories.push_back(history->state_histories[i_index]);
			this->existing_signal_histories.push_back(history->signal_histories[i_index]);
			this->existing_target_val_histories.push_back(target_val);
		}

		this->sum_vals += target_val;

		this->state_iter++;
		if (this->state_iter >= EXPERIMENT_TRAIN_EXISTING_NUM_DATAPOINTS) {
			this->existing_val_average = this->sum_vals / this->state_iter;

			train_existing_helper();

			explore_helper();
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->best_surprise >= 0.0) {
			#endif /* MDEBUG */
				train_new_helper(wrapper);
			} else {
				delete this;
			}
		}
	}
}
