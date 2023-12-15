#include "connection_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "world_model.h"
#include "world_state.h"

using namespace std;

void ConnectionExperiment::verify_activate(WorldState*& curr_state) {
	curr_state = this->target;
}

void ConnectionExperiment::verify_backprop(double target_val,
										   WorldState* ending_state,
										   vector<double>& ending_state_vals) {
	double predicted_score = ending_state->val_average;
	for (int s_index = 0; s_index < world_model->num_states; s_index++) {
		predicted_score += ending_state->state_val_impacts[s_index] * ending_state_vals[s_index];
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
			if (this->is_obs) {
				/**
				 * - has to be new obs_transition
				 */
				this->parent->obs_transitions.push_back(this->target);
				this->parent->obs_indexes.push_back(this->obs_index);
				this->parent->obs_is_greater.push_back(this->obs_is_greater);
			} else {
				int t_index = 0;
				while (true) {
					if (t_index >= (int)this->parent->action_transitions.size()) {
						break;
					}

					bool is_match = true;
					if (this->parent->action_transition_actions[t_index] != this->action) {
						is_match = false;
					} else {
						for (int s_index = 0; s_index < (int)this->action_states.size(); s_index++) {
							bool is_input_match = false;
							for (int is_index = 0; is_index < (int)this->parent->action_transition_states[t_index].size(); is_index++) {
								if (this->parent->action_transition_states[t_index][is_index] == this->action_states[s_index]) {
									is_input_match = true;
									break;
								}
							}

							if (!is_input_match) {
								is_match = false;
								break;
							}
						}
					}

					if (is_match) {
						this->parent->action_transitions.erase(this->parent->action_transitions.begin() + t_index);
						this->parent->action_transition_actions.erase(this->parent->action_transition_actions.begin() + t_index);
						this->parent->action_transition_states.erase(this->parent->action_transition_states.begin() + t_index);
					} else {
						t_index++;
					}
				}

				/**
				 * - insert at front to take priority
				 */
				this->parent->action_transitions.insert(this->parent->action_transitions.begin(), this->target);
				this->parent->action_transition_actions.insert(this->parent->action_transition_actions.begin(), this->action);
				this->parent->action_transition_states.insert(this->parent->action_transition_states.begin(), this->parent->action_experiments[this->action].first);
			}

			cout << "connection_experiment" << endl;
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
