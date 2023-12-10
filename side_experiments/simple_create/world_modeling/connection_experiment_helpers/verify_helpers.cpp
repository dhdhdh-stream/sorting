#include "connection_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "hidden_state.h"

using namespace std;

void ConnectionExperiment::verify_activate(HiddenState*& curr_state,
										   vector<int>& action_sequence) {
	curr_state = this->target;
}

void ConnectionExperiment::verify_backprop(double target_val,
										   HiddenState* ending_state) {
	double curr_misguess = (target_val - ending_state->average_val)*(target_val - ending_state->average_val);
	this->new_misguess += curr_misguess;

	this->state_iter++;
	if (this->state_iter >= 2 * MEASURE_NUM_SAMPLES) {
		this->new_misguess /= (2 * MEASURE_NUM_SAMPLES);

		double misguess_standard_deviation = sqrt(this->existing_misguess_variance);
		double new_improvement = this->existing_average_misguess - this->new_misguess;
		double new_improvement_t_score = new_improvement
			/ (misguess_standard_deviation / sqrt(2 * MEASURE_NUM_SAMPLES));

		cout << "this->new_misguess: " << this->new_misguess << endl;
		cout << "new_improvement_t_score: " << new_improvement_t_score << endl;

		if (new_improvement_t_score > 2.326) {
			map<int, AbstractExperiment*>::iterator it = this->parent->experiments.begin();
			while (true) {
				if (it->second == this) {
					break;
				}
				it++;
			}
			this->parent->transitions[it->first] = this->target;

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
