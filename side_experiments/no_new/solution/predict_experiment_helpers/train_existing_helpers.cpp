#include "predict_experiment.h"

#include "constants.h"
#include "globals.h"
#include "score_network.h"

using namespace std;

void PredictExperiment::train_existing_helper() {
	this->existing_network = new ScoreNetwork(this->existing_state_histories[0].size());

	uniform_int_distribution<int> train_distribution(0, this->existing_state_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = train_distribution(generator);

		this->existing_network->activate(this->existing_state_histories[rand_index]);

		if (this->use_signal) {
			this->existing_network->init_backprop(this->existing_signal_histories[rand_index]);
		} else {
			this->existing_network->init_backprop(this->existing_target_val_histories[rand_index]);
		}

		if ((iter_index+1)%INIT_EPOCH_SIZE == 0) {
			this->existing_network->init_update();
		}
	}
	for (int s_index = 0; s_index < (int)this->existing_network->state_input->errors.size(); s_index++) {
		this->existing_network->state_input->errors(s_index) = 0.0;
	}
}
