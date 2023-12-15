#include "connection_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "world_model.h"
#include "world_state.h"

using namespace std;

void ConnectionExperiment::measure_activate(WorldState*& curr_state) {
	curr_state = this->target;
}

void ConnectionExperiment::measure_backprop(double target_val,
											WorldState* ending_state,
											vector<double>& ending_state_vals) {
	double predicted_score = ending_state->val_average;
	for (int s_index = 0; s_index < world_model->num_states; s_index++) {
		predicted_score += ending_state->state_val_impacts[s_index] * ending_state_vals[s_index];
	}

	double curr_misguess = (target_val - predicted_score)*(target_val - predicted_score);
	this->new_misguess += curr_misguess;

	this->state_iter++;
	if (this->state_iter >= MEASURE_NUM_SAMPLES) {
		this->new_misguess /= MEASURE_NUM_SAMPLES;

		double misguess_standard_deviation = sqrt(this->existing_misguess_variance);
		double new_improvement = this->existing_average_misguess - this->new_misguess;
		double new_improvement_t_score = new_improvement
			/ (misguess_standard_deviation / sqrt(MEASURE_NUM_SAMPLES));

		if (new_improvement_t_score > 2.326) {
			this->new_misguess = 0.0;

			this->misguess_histories.reserve(MEASURE_NUM_SAMPLES);

			this->state = CONNECTION_EXPERIMENT_STATE_VERIFY_EXISTING;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
