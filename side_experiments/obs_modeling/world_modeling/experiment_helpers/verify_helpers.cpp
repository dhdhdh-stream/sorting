#include "experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "world_state.h"
#include "hmm.h"

using namespace std;

void Experiment::verify_activate(WorldState*& curr_state) {
	curr_state = this->experiment_states[0];
}

void Experiment::verify_backprop(double target_val,
								 WorldState* ending_state,
								 vector<double>& ending_state_vals) {
	double predicted_score;
	map<WorldState*, int>::iterator it = this->experiment_state_reverse_mapping.find(ending_state);
	if (it != this->experiment_state_reverse_mapping.end()) {
		predicted_score = this->ending_val_averages[it->second];
		for (int s_index = 0; s_index < world_model->num_states; s_index++) {
			predicted_score += this->ending_state_val_impacts[it->second][s_index] * ending_state_vals[s_index];
		}
	} else {
		predicted_score = ending_state->val_average;
		for (int s_index = 0; s_index < world_model->num_states; s_index++) {
			predicted_score += ending_state->state_val_impacts[s_index] * ending_state_vals[s_index];
		}
	}

	double curr_misguess = (target_val - predicted_score)*(target_val - predicted_score);
	this->new_misguess += curr_misguess;

	this->state_iter++;
	if (this->state_iter >= 2 * MEASURE_NUM_SAMPLES) {
		this->new_misguess /= (2 * MEASURE_NUM_SAMPLES);

		double misguess_standard_deviation = sqrt(this->existing_misguess_variance);
		double new_improvement = this->existing_average_misguess - this->new_misguess;
		double new_improvement_t_score = new_improvement
			/ (misguess_standard_deviation / sqrt(2 * MEASURE_NUM_SAMPLES));

		if (new_improvement_t_score > 2.326) {
			for (int a_index = 0; a_index < (int)this->experiment_states.size(); a_index++) {
				this->experiment_states[a_index]->val_average = this->ending_val_averages[a_index];
				this->experiment_states[a_index]->state_val_impacts = this->ending_state_val_impacts[a_index];
			}

			if (this->is_obs) {
				this->parent->obs_transitions.push_back(this->experiment_states[0]);
				this->parent->obs_indexes.push_back(this->obs_index);
				this->parent->obs_is_greater.push_back(this->obs_is_greater);
			} else {
				this->parent->action_transitions.push_back(this->experiment_states[0]);
				this->parent->action_transition_actions.push_back(this->action);
				this->parent->action_transition_states.push_back(
					this->parent->action_experiments[this->action].first);
			}

			for (int s_index = 0; s_index < (int)this->experiment_states.size(); s_index++) {
				this->experiment_states[s_index]->id = (int)hmm->hidden_states.size();
				hmm->hidden_states.push_back(this->experiment_states[s_index]);
			}
			this->experiment_states.clear();

			cout << "this->existing_average_misguess: " << this->existing_average_misguess << endl;
			cout << "this->existing_misguess_variance: " << this->existing_misguess_variance << endl;
			cout << "this->new_misguess: " << this->new_misguess << endl;
			cout << "new_improvement_t_score: " << new_improvement_t_score << endl;

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
