#include "experiment.h"

#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void Experiment::train_existing_check_activate(
		vector<double>& obs,
		SolutionWrapper* wrapper,
		ExperimentHistory* history) {
	this->obs_histories.push_back(obs);

	this->target_val_histories.push_back(wrapper->prev_clean_result);
}

void Experiment::train_existing_backprop(double target_val,
										 ExperimentHistory* history,
										 SolutionWrapper* wrapper) {
	this->state_iter++;
	if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_network = new Network(this->obs_histories[0].size());
		uniform_int_distribution<int> new_input_distribution(0, this->obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = new_input_distribution(generator);

			this->existing_network->activate(this->obs_histories[rand_index]);

			double error = this->target_val_histories[rand_index] - this->existing_network->output->acti_vals[0];

			this->existing_network->backprop(error);
		}

		this->obs_histories.clear();
		this->target_val_histories.clear();

		this->curr_surprise = 0.0;
		this->curr_new_scope = NULL;
		this->best_surprise = 0.0;
		this->best_new_scope = NULL;

		this->state = EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
