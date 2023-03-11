#include "fold.h"

using namespace std;

void Fold::clear_inner_state_end() {
	if (this->sum_error/this->sequence_length / this->sub_state_iter < 0.01) {
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->curr_state_networks[f_index].size(); s_index++) {
				if (this->curr_state_networks[f_index][s_index] != NULL) {
					delete this->curr_state_networks[f_index][s_index];
				}
			}

			delete this->curr_score_networks[f_index];
		}
		this->curr_state_networks = this->test_state_networks;
		this->curr_score_networks = this->test_score_networks;

		this->curr_num_states_cleared = this->test_num_states_cleared;

		this->test_num_states_cleared[this->clean_inner_step_index]++;
		if (this->test_num_states_cleared[this->clean_inner_step_index] > this->sum_inner_inputs+this->curr_num_new_inner_states) {
			this->clean_inner_step_index++;
			if (this->clean_inner_step_index >= this->sequence_length) {
				this->state = FOLD_STATE_DONE;
			} else {
				this->clean_inner_state_index = 0;

				this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
				this->test_state_networks_not_needed[this->clean_inner_step_index][0] = true;

				int curr_total_num_states = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_sequence_local_states
					+ this->num_sequence_input_states
					+ this->curr_num_new_outer_states;
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					for (int s_index = 0; s_index < curr_total_num_states; s_index++) {
						if (!this->test_state_networks_not_needed[f_index][s_index]) {
							this->test_state_networks[f_index][s_index] = new StateNetwork(this->curr_state_networks[f_index][s_index]);
						} else {
							this->test_state_networks[f_index][s_index] = NULL;
						}
					}

					this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
				}

				this->state = FOLD_STATE_REMOVE_INNER_NETWORK;
				this->state_iter = 0;
				this->sub_state_iter = 0;
				this->sum_error = 0.0;
			}
		} else {
			int total_num_states = this->sum_inner_inputs
				+ this->curr_num_new_inner_states
				+ this->num_sequence_local_states
				+ this->num_sequence_input_states
				+ this->curr_num_new_outer_states;
			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				for (int s_index = 0; s_index < total_num_states; s_index++) {
					if (!this->curr_state_networks_not_needed[f_index][s_index]) {
						this->test_state_networks[f_index][s_index] = new StateNetwork(this->curr_state_networks[f_index][s_index]);
					} else {
						this->test_state_networks[f_index][s_index] = NULL;
					}
				}

				this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
			}

			this->state = FOLD_STATE_CLEAR_INNER_STATE;
			this->state_iter = 0;
			this->sub_state_iter = 0;
			this->sum_error = 0.0;
		}
	} else {
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->test_state_networks[f_index].size(); s_index++) {
				if (this->test_state_networks[f_index][s_index] != NULL) {
					delete this->test_state_networks[f_index][s_index];
				}
			}

			delete this->test_score_networks[f_index];
		}

		this->clean_inner_step_index++;
		if (this->clean_inner_step_index >= this->sequence_length) {
			this->state = FOLD_STATE_DONE;
		} else {
			this->clean_inner_state_index = 0;

			this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
			this->test_state_networks_not_needed[this->clean_inner_step_index][0] = true;

			int curr_total_num_states = this->sum_inner_inputs
				+ this->curr_num_new_inner_states
				+ this->num_sequence_local_states
				+ this->num_sequence_input_states
				+ this->curr_num_new_outer_states;
			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				for (int s_index = 0; s_index < curr_total_num_states; s_index++) {
					if (!this->test_state_networks_not_needed[f_index][s_index]) {
						this->test_state_networks[f_index][s_index] = new StateNetwork(this->curr_state_networks[f_index][s_index]);
					} else {
						this->test_state_networks[f_index][s_index] = NULL;
					}
				}

				this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
			}

			this->state = FOLD_STATE_REMOVE_INNER_NETWORK;
			this->state_iter = 0;
			this->sub_state_iter = 0;
			this->sum_error = 0.0;
		}
	}
}
