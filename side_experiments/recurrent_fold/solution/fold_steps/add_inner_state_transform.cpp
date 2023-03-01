#include "fold.h"

using namespace std;

void Fold::add_inner_state_end() {
	// TODO: check for score increase or misguess improvement



	this->test_num_new_outer_states = 0;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
			this->test_state_networks[f_index].push_back(this->curr_state_networks[f_index][i_index]);
			this->test_state_networks[f_index].back()->remove_new_outer();
		}

		this->test_score_networks[f_index] = this->curr_score_networks[f_index];
		this->test_score_networks[f_index]->remove_new_outer();
	}

	this->state = STATE_REMOVE_OUTER_STATE;
	this->state_iter = 0;
	this->sum_error = 0.0;
}
