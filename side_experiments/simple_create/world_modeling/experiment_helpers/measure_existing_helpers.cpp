#include "experiment.h"

#include <iostream>

#include "constants.h"
#include "hidden_state.h"

using namespace std;

void Experiment::measure_existing_backprop(double target_val,
										   HiddenState* ending_state) {
	double curr_misguess = (target_val - ending_state->average_val)*(target_val - ending_state->average_val);
	this->misguess_histories.push_back(curr_misguess);

	if ((int)this->misguess_histories.size() >= MEASURE_NUM_SAMPLES) {
		double sum_misguesses = 0.0;
		for (int d_index = 0; d_index < MEASURE_NUM_SAMPLES; d_index++) {
			sum_misguesses += this->misguess_histories[d_index];
		}
		this->existing_average_misguess = sum_misguesses / MEASURE_NUM_SAMPLES;

		double sum_variance = 0.0;
		for (int d_index = 0; d_index < MEASURE_NUM_SAMPLES; d_index++) {
			sum_variance += (this->misguess_histories[d_index] - this->existing_average_misguess) * (this->misguess_histories[d_index] - this->existing_average_misguess);
		}
		this->existing_misguess_variance = sum_variance / MEASURE_NUM_SAMPLES;

		cout << "this->existing_average_misguess: " << this->existing_average_misguess << endl;
		cout << "this->existing_misguess_variance: " << this->existing_misguess_variance << endl;

		this->misguess_histories.clear();

		for (int s_index = 0; s_index < (int)this->experiment_states.size(); s_index++) {
			this->ending_state_vals[this->experiment_states[s_index]] = vector<double>();
		}

		this->state = EXPERIMENT_STATE_TRAIN;
		this->state_iter = 0;
	}
}
