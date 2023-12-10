#include "experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "hidden_state.h"
#include "hmm.h"

using namespace std;

void Experiment::verify_activate(HiddenState*& curr_state,
								 vector<int>& action_sequence) {
	// action_sequence[0] == this->starting_action;
	action_sequence.erase(action_sequence.begin());
	curr_state = this->experiment_states[0];
}

void Experiment::verify_backprop(double target_val,
								 HiddenState* ending_state) {
	map<HiddenState*, double>::iterator it = this->ending_state_means.find(ending_state);
	if (it != this->ending_state_means.end()) {
		double curr_misguess = (target_val - it->second)*(target_val - it->second);
		this->new_misguess += curr_misguess;
	} else {
		double curr_misguess = (target_val - ending_state->average_val)*(target_val - ending_state->average_val);
		this->new_misguess += curr_misguess;
	}

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
			for (map<HiddenState*, double>::iterator it = this->ending_state_means.begin();
					it != this->ending_state_means.end(); it++) {
				it->first->average_val = it->second;
			}

			this->parent->transitions[this->starting_action] = this->experiment_states[0];

			for (int s_index = 0; s_index < (int)this->experiment_states.size(); s_index++) {
				hmm->hidden_states.push_back(this->experiment_states[s_index]);
			}
			this->experiment_states.clear();

			this->state = EXPERIMENT_STATE_SUCCESS;
		} else {
			this->state = EXPERIMENT_STATE_FAIL;
		}
	}
}
