#include "force_experiment.h"

#include "constants.h"
#include "experiment_run.h"
#include "globals.h"
#include "network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_SAMPLES = 10;
#else
const int TRAIN_EXISTING_NUM_SAMPLES = 4000;
#endif /* MDEBUG */

void ForceExperiment::train_existing_experiment_activate(ExperimentRun* run) {
	ForceExperimentHistory* history = run->force_experiment_histories[this];
	history->state = run->state;
}

void ForceExperiment::train_existing_backprop(double target_val,
											  ExperimentRun* run,
											  ForceExperimentHistory* history,
											  Wrapper* wrapper) {
	this->existing_states.push_back(history->state);
	this->existing_target_vals.push_back(target_val);

	this->state_iter++;
	if (this->state_iter >= TRAIN_EXISTING_NUM_SAMPLES) {
		this->original_network = new Network(this->existing_states[0].size());
		double existing_hidden_1_average_max_update = 0.0;
		double existing_hidden_2_average_max_update = 0.0;
		double existing_hidden_3_average_max_update = 0.0;
		double existing_output_average_max_update = 0.0;
		uniform_int_distribution<int> distribution(0, this->existing_states.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = distribution(generator);
			this->original_network->activate(this->existing_states[index]);
			double error = this->existing_target_vals[index] - this->original_network->output->acti_vals[0];
			this->original_network->init_backprop(error,
												  existing_hidden_1_average_max_update,
												  existing_hidden_2_average_max_update,
												  existing_hidden_3_average_max_update,
												  existing_output_average_max_update);
		}

		this->best_surprise = 0.0;

		this->state = FORCE_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
