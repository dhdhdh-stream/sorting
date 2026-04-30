#include "experiment.h"

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void Experiment::train_existing_check_activate(vector<double>& obs) {
	this->existing_obs_histories.push_back(obs);
}

void Experiment::train_existing_backprop(double target_val,
										 ExperimentHistory* history,
										 SolutionWrapper* wrapper) {
	while (this->existing_target_val_histories.size() < this->existing_obs_histories.size()) {
		this->existing_target_val_histories.push_back(target_val);
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_network = new Network(this->existing_obs_histories[0].size());

		uniform_int_distribution<int> distribution(0, this->existing_obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = distribution(generator);

			this->existing_network->activate(this->existing_obs_histories[rand_index]);

			double error = this->existing_target_val_histories[rand_index] - this->existing_network->output->acti_vals[0];

			this->existing_network->backprop(error);
		}

		this->existing_obs_histories.clear();
		this->existing_target_val_histories.clear();

		this->curr_explore_count = 0;
		this->best_surprise = 0.0;

		this->state = EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
