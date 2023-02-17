#include "fold.h"

using namespace std;

Fold::Fold(int sequence_length) {
	this->sequence_length = sequence_length;

	this->curr_num_states = 0;
	this->curr_state_networks = vector<vector<Network*>>(this->sequence_length, vector<Network*>());
	this->curr_score_networks = vector<Network*>(this->sequence_length, new Network(0, 20, 1));

	add_state();
}

Fold::~Fold() {
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < (int)this->curr_state_networks[f_index].size(); s_index++) {
			delete this->curr_state_networks[f_index][s_index];
		}

		delete this->curr_score_networks[f_index];
	}
}

void Fold::activate(vector<vector<double>>& flat_vals,
					double predicted_score) {
	// TODO: clean up later
	vector<double> state_vals(this->curr_num_states, 0.0);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
			vector<double> state_network_input = flat_vals.begin();
			for (int i_index = 0; i_index < s_index+1; i_index++) {
				state_network_input.push_back(state_vals[i_index]);
			}
			this->curr_state_networks[f_index][s_index]->activate(state_network_input);
			state_vals[s_index] += this->curr_state_networks[f_index][s_index]->output->acti_vals[0];
		}

		vector<double> score_network_input = flat_vals.begin();
		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
			score_network_input.push_back(state_vals[i_index]);
		}
		this->curr_score_networks[f_index]->activate(score_network_input);
		predicted_score += this->curr_score_networks[f_index]->output->acti_vals[0];

		flat_vals.erase(flat_vals.begin());
	}
}

void Fold::backprop(double target_val) {
	double outer_error = target_val - this->outer_network->output->acti_vals[0];
	this->sum_error += abs(outer_error);
	vector<double> outer_errors{outer_error};
	this->outer_network->backprop(outer_errors, 0.01);

	vector<double> state_errors(this->num_states);
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		state_errors[s_index] = this->outer_network->input->errors[s_index];
		this->outer_network->input->errors[s_index] = 0.0;
	}

	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
		for (int s_index = this->num_states-1; s_index >= 0; s_index--) {
			vector<double> state_network_output_errors{state_errors[s_index]};
			this->state_networks[f_index][s_index]->backprop(state_network_output_errors, 0.01);
			for (int i_index = 0; i_index < s_index+1; i_index++) {
				state_errors[i_index] += this->state_networks[f_index][s_index]->input->errors[i_index];
				this->state_networks[f_index][s_index]->input->errors[i_index] = 0.0;
			}
		}
	}
}

void Fold::add_state() {
	this->curr_num_states++;
	
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		int new_input_size = 1 + this->num_states;	// flat size always 1 for now
		this->state_networks[f_index].push_back(new Network(new_input_size,
															20,
															1));

		this->curr_score_networks[f_index]->add_input();
	}

	// TODO: initialize test_state_networks, test_score_networks
}

void Fold::flat_to_fold() {
	this->fold_step_index = 0;
	this->fold_state_index = 0;

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		this->curr_state_networks_not_needed.push_back(vector<bool>(this->curr_num_states, false));
		this->test_state_networks_not_needed.push_back(vector<bool>(this->curr_num_states, false));
		this->curr_state_not_needed_locally.push_back(vector<bool>(this->curr_num_states, false));
		this->test_state_not_needed_locally.push_back(vector<bool>(this->curr_num_states, false));
		this->curr_num_states_cleared.push_back(0);
		this->test_num_states_cleared.push_back(0);
	}

	this->state = STATE_REMOVE_NETWORK;

	this->test_state_networks_not_needed[0][0] = true;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
			if (!this->test_state_networks_not_needed[f_index][s_index]) {
				this->test_state_networks[f_index][s_index] = new Network(this->curr_state_networks[f_index][s_index]);
			} else {
				this->test_state_networks[f_index][s_index] = NULL;
			}
		}

		this->test_score_networks[f_index] = new Network(this->curr_score_networks[f_index]);
	}

	this->state_iter = 0;
	this->sum_error = 0.0;
}

void Fold::fold_activate(vector<vector<double>>& flat_vals,
						 double& predicted_score) {
	vector<double> curr_state_vals(this->curr_num_states, 0.0);
	vector<double> test_state_vals(this->curr_num_states, 0.0);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
			if (!this->curr_state_networks_not_needed[f_index][s_index]) {
				vector<double> state_network_input = flat_vals.begin();
				for (int i_index = 0; i_index < s_index+1; i_index++) {
					if (this->curr_state_not_needed_locally[f_index][i_index]) {
						state_network_input.push_back(0.0);
					} else {
						state_network_input.push_back(curr_state_vals[i_index]);
					}
				}
				this->curr_state_networks[f_index][s_index]->activate(state_network_input);
				curr_state_vals[s_index] += this->curr_state_networks[f_index][s_index]->output->acti_vals[0];
			}
		}
		for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
			if (!this->test_state_networks_not_needed[f_index][s_index]) {
				vector<double> state_network_input = flat_vals.begin();
				for (int i_index = 0; i_index < s_index+1; i_index++) {
					if (this->test_state_not_needed_locally[f_index][i_index]) {
						state_network_input.push_back(0.0);
					} else {
						state_network_input.push_back(test_state_vals[i_index]);
					}
				}
				this->test_state_networks[f_index][s_index]->activate(state_network_input);
				test_state_vals[s_index] += this->test_state_networks[f_index][s_index]->output->acti_vals[0];
			}
		}

		vector<double> curr_score_network_input = flat_vals.begin();
		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
			if (this->curr_state_not_needed_locally[f_index][i_index]) {
				score_network_input.push_back(0.0);
			} else {
				score_network_input.push_back(curr_state_vals[i_index]);
			}
		}
		this->curr_score_networks[f_index]->activate(curr_score_network_input);
		predicted_score += this->curr_score_networks[f_index]->output->acti_vals[0];

		vector<double> test_score_network_input = flat_vals.begin();
		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
			if (this->test_state_not_needed_locally[f_index][i_index]) {
				test_score_network_input.push_back(0.0);
			} else {
				test_score_network_input.push_back(test_state_vals[i_index]);
			}
		}
		this->test_score_networks[f_index]->activate(test_score_network_input);

		for (int s_index = 0; s_index < this->curr_num_states_cleared[f_index]; s_index++) {
			curr_state_vals[s_index] = 0.0;
		}
		for (int s_index = 0; s_index < this->test_num_states_cleared[f_index]; s_index++) {
			test_state_vals[s_index] = 0.0;
		}

		flat_vals.erase(flat_vals.begin());
	}
}

void Fold::fold_backprop(double target_val,
						 double& predicted_score) {
	vector<double> curr_state_errors(this->curr_num_states, 0.0);
	vector<double> test_state_errors(this->curr_num_states, 0.0);

	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
		vector<double> test_score_network_errors{this->curr_score_networks[f_index]->output->acti_vals[0]
			- this->test_score_networks[f_index]->output->acti_vals[0]};
		if (this->state_iter <= 130000) {
			this->test_score_networks[f_index]->backprop(test_score_network_errors, 0.01);
		} else {
			this->test_score_networks[f_index]->backprop(test_score_network_errors, 0.002);
		}
		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
			if (!this->test_state_not_needed_locally[f_index][i_index]) {
				test_state_errors[i_index] += this->test_score_networks[f_index]->input->errors[i_index];
				this->test_score_networks[f_index]->input->errors[i_index] = 0.0;
			}
		}

		vector<double> curr_score_network_errors{target_val - predicted_score};
		this->curr_score_networks[f_index]->backprop(curr_score_network_errors, 0.002);
		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
			if (!this->curr_state_not_needed_locally[f_index][i_index]) {
				curr_state_errors[i_index] += this->curr_score_networks[f_index]->input->errors[i_index];
				this->curr_score_networks[f_index]->input->errors[i_index] = 0.0;
			}
		}

		predicted_score -= this->curr_score_networks[f_index]->output->acti_vals[0];

		for (int s_index = this->curr_num_states-1; s_index >= 0; s_index--) {
			if (!this->test_state_networks_not_needed[f_index][s_index]) {
				vector<double> state_network_errors{test_state_errors[s_index]};
				if (this->state_iter <= 130000) {
					this->test_state_networks[f_index][s_index]->backprop(state_network_errors, 0.01);
				} else {
					this->test_state_networks[f_index][s_index]->backprop(state_network_errors, 0.002);
				}
				for (int i_index = 0; i_index < s_index+1; i_index++) {
					if (!this->test_state_not_needed_locally[f_index][i_index]) {
						test_state_errors[i_index] += this->test_state_networks[f_index]->input->errors[i_index];
						this->test_state_networks[f_index]->input->errors[i_index] = 0.0;
					}
				}
			}
		}

		for (int s_index = this->curr_num_states-1; s_index >= 0; s_index--) {
			if (!this->curr_state_networks_not_needed[f_index][s_index]) {
				vector<double> state_network_errors{curr_state_errors[s_index]};
				this->curr_state_networks[f_index][s_index]->backprop(state_network_errors, 0.002);
				for (int i_index = 0; i_index < s_index+1; i_index++) {
					if (!this->curr_state_not_needed_locally[f_index][i_index]) {
						curr_state_errors[i_index] += this->curr_state_networks[f_index]->input->errors[i_index];
						this->curr_state_networks[f_index]->input->errors[i_index] = 0.0;
					}
				}
			}
		}
	}

	fold_increment();
}

void Fold::fold_increment() {
	this->state_iter++;

	if (this->state_iter == 150000) {
		if (this->sum_error/this->sequence_length/10000 < 0.001) {
			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
					if (!this->curr_state_networks_not_needed[f_index][s_index]) {
						delete this->curr_state_networks[f_index][s_index];
					}
				}

				delete this->curr_score_networks[f_index];
			}

			this->curr_state_networks_not_needed = this->test_state_networks_not_needed;
			this->curr_state_not_needed_locally = this->test_state_not_needed_locally;
			this->curr_num_states_cleared = this->test_num_states_cleared;
			this->curr_state_networks = this->test_state_networks;
			this->curr_score_networks = this->test_score_networks;
		} else {
			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
					if (!this->test_state_networks_not_needed[f_index][s_index]) {
						delete this->test_state_networks[f_index][s_index];
					}
				}

				delete this->test_score_networks[f_index];
			}
		}

		this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
		this->test_state_not_needed_locally = this->curr_state_not_needed_locally;
		this->test_num_states_cleared = this->curr_num_states_cleared;

		bool next_step_done = false;
		while (!next_step_done) {
			if (this->state == STATE_REMOVE_NETWORK) {
				if (this->fold_state_index == this->curr_num_states-1) {
					this->state = STATE_REMOVE_STATE;
					this->fold_state_index = 0;

					if (this->curr_state_networks_not_needed[this->fold_step_index][this->fold_state_index]) {
						next_step_done = true;
						this->test_state_not_needed_locally[this->fold_step_index][this->fold_state_index] = true;
					}
				} else {
					this->fold_state_index++;

					next_step_done = true;
					this->test_state_networks_not_needed[this->fold_step_index][this->fold_state_index] = true;
				}
			} else if (this->state == STATE_REMOVE_STATE) {
				if (this->fold_state_index == this->curr_num_states-1) {
					this->state = STATE_CLEAR_STATE;
					
					if (this->fold_step_index == 0) {
						// do nothing -- this->curr_num_states_cleared[0] = 0
					} else {
						this->curr_num_states_cleared[this->fold_step_index] = this->curr_num_states_cleared[this->fold_step_index-1];
						for (int s_index = 0; s_index < this->curr_num_states_cleared[this->fold_step_index]; s_index++) {
							if (!this->curr_state_networks_not_needed[this->fold_step_index][s_index]) {
								this->curr_num_states_cleared[this->fold_step_index] = s_index;
								break;
							}
						}
					}

					this->fold_state_index = this->curr_num_states_cleared[this->fold_step_index];

				} else {
					this->fold_state_index++;

					if (this->curr_state_networks_not_needed[this->fold_step_index][this->fold_state_index]) {
						next_step_done = true;
						this->test_state_not_needed_locally[this->fold_step_index][this->fold_state_index] = true;
					}
				}
			} else {
				// this->state == STATE_CLEAR_STATE
				if (this->fold_state_index == this->curr_num_states-1) {
					if ()
				}

				// if next step, reset num_states_cleared
			}
		}

		if (!this->state == STATE_DONE) {
			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
					if (!this->test_state_networks_not_needed[f_index][s_index]) {
						this->test_state_networks[f_index][s_index] = new Network(this->curr_state_networks[f_index][s_index]);
					} else {
						this->test_state_networks[f_index][s_index] = NULL;
					}
				}

				this->test_score_networks[f_index] = new Network(this->curr_score_networks[f_index]);
			}
		}

		this->state_iter = 0;
		this->sum_error = 0.0;
	} else {
		if (this->state_iter%10000 == 0) {
			cout << "this->state_iter: " << this->state_iter << endl;
			cout << "this->sum_error: " << this->sum_error << endl;
			cout << endl;
			this->sum_error = 0.0;
		}
	}
}

void Fold::
