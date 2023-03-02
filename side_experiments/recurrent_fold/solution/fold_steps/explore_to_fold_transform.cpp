#include "fold.h"

using namespace std;

void Fold::explore_to_fold() {
	// don't special case outer

	this->test_num_new_inner_states = this->curr_num_new_inner_states+1;
	int curr_total_num_states = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_local_states
		+ this->num_input_states
		+ this->curr_num_new_outer_states;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
			this->test_state_networks[f_index].push_back(new StateNetwork(
				this->curr_state_networks[f_index][i_index]));
			this->test_state_networks[f_index][i_index]->add_new_inner();
		}
		if (this->is_inner_scope[f_index]) {
			this->test_state_networks[f_index].push_back(new StateNetwork(0,
																		  this->num_local_states,
																		  this->num_input_states,
																		  this->sum_inner_inputs+this->test_num_new_inner_states,
																		  this->curr_num_new_outer_states,
																		  20));
		} else {
			this->test_state_networks[f_index].push_back(new StateNetwork(1,
																		  this->num_local_states,
																		  this->num_input_states,
																		  this->sum_inner_inputs+this->test_num_new_inner_states,
																		  this->curr_num_new_outer_states,
																		  20));
		}
		for (int s_index = this->sum_inner_inputs+this->curr_num_new_inner_states; s_index < curr_total_num_states; s_index++) {
			this->test_state_networks[f_index].push_back(new StateNetwork(
				this->curr_state_networks[f_index][s_index]));
			this->test_state_networks[f_index][s_index]->add_new_inner();
		}

		this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
		this->test_score_networks[f_index]->add_new_inner();
	}

	this->state = STATE_ADD_INNER_STATE;
	this->state_iter = 0;
	this->sum_error = 0.0;
}
