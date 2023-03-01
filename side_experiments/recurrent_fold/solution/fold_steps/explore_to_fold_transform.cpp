#include "fold.h"

using namespace std;

void Fold::explore_to_fold() {
	this->test_num_new_outer_states = this->curr_num_new_outer_states;
	for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
			it != this->curr_outer_state_networks.end(); it++) {
		this->test_outer_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			// this->curr_num_new_outer_states = 1
			this->test_outer_state_networks[it->first].push_back(vector<StateNetwork*>());
			this->test_outer_state_networks[it->first].back().push_back(
				new StateNetwork(it->second[n_index][0]));
		}
	}

	this->test_starting_score_network = new StateNetwork(this->curr_starting_score_network);

	this->test_num_new_inner_states = this->curr_num_new_inner_states+1;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
			this->test_state_networks[f_index].push_back(this->curr_state_networks[f_index][i_index]);
			this->test_state_networks[f_index].back()->add_new_inner();
		}

		this->test_score_networks[f_index] = this->curr_score_networks[f_index];
		this->test_score_networks[f_index]->add_new_inner();
	}

	this->state = STATE_ADD_INNER_STATE;
	this->state_iter = 0;
	this->sum_error = 0.0;
}
