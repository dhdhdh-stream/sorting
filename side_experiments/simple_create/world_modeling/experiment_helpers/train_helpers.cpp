#include "experiment.h"

#include "constants.h"
#include "hidden_state.h"

using namespace std;

const int TRAIN_NUM_SAMPLES = 2000;

void Experiment::train_activate(HiddenState* curr_state,
								vector<int>& action_sequence) {
	// action_sequence[0] == this->starting_action;
	action_sequence.erase(action_sequence.begin());
	curr_state = this->experiment_states[0];
}

void Experiment::train_backprop(double target_val,
								HiddenState* ending_state) {
	map<HiddenState*, vector<double>>::iterator it = this->ending_state_vals.find(ending_state);
	if (it != this->ending_state_vals.end()) {
		it->second.push_back(target_val);
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_NUM_SAMPLES) {
		for (map<HiddenState*, vector<double>>::iterator it = this->ending_state_vals.begin();
				it != this->ending_state_vals.end(); it++) {
			if (it->second.size() == 0) {
				this->ending_state_means[it->first] = 0.0;
			} else {
				double sum_vals = 0.0;
				for (int d_index = 0; d_index < (int)it->second.size(); d_index++) {
					sum_vals += it->second[d_index];
				}
				this->ending_state_means[it->first] = sum_vals / it->second.size();
			}
		}

		this->misguess_histories.reserve(MEASURE_NUM_SAMPLES);

		this->state = EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
	}
}
