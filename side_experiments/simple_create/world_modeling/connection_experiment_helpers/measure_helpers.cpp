#include "connection_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "hidden_state.h"

using namespace std;

void ConnectionExperiment::measure_activate(HiddenState*& curr_state,
											vector<int>& action_sequence) {
	curr_state = this->target;
}

void ConnectionExperiment::measure_backprop(double target_val,
											HiddenState* ending_state) {
	double curr_misguess = (target_val - ending_state->average_val)*(target_val - ending_state->average_val);
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
