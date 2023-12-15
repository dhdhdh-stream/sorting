#include "experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "world_model.h"
#include "world_state.h"

using namespace std;

void Experiment::verify_existing_backprop(double target_val,
										  WorldState* ending_state,
										  vector<double>& ending_state_vals) {
	double predicted_score = ending_state->val_average;
	for (int s_index = 0; s_index < world_model->num_states; s_index++) {
		predicted_score += ending_state->state_val_impacts[s_index] * ending_state_vals[s_index];
	}

	double curr_misguess = (target_val - predicted_score)*(target_val - predicted_score);
	this->misguess_histories.push_back(curr_misguess);

	if ((int)this->misguess_histories.size() >= 2 * MEASURE_NUM_SAMPLES) {
		double sum_misguesses = 0.0;
		for (int d_index = 0; d_index < 2 * MEASURE_NUM_SAMPLES; d_index++) {
			sum_misguesses += this->misguess_histories[d_index];
		}
		this->existing_average_misguess = sum_misguesses / (2 * MEASURE_NUM_SAMPLES);

		double sum_variance = 0.0;
		for (int d_index = 0; d_index < 2 * MEASURE_NUM_SAMPLES; d_index++) {
			sum_variance += (this->misguess_histories[d_index] - this->existing_average_misguess) * (this->misguess_histories[d_index] - this->existing_average_misguess);
		}
		this->existing_misguess_variance = sum_variance / (2 * MEASURE_NUM_SAMPLES);

		this->misguess_histories.clear();

		this->state = EXPERIMENT_STATE_VERIFY;
		this->state_iter = 0;
	}
}
