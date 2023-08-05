#include "score_network.h"

#include <iostream>

using namespace std;

void ScoreNetwork::construct() {
	this->state_input = new Layer(LINEAR_LAYER, this->state_size);
	this->new_state_input = new Layer(LINEAR_LAYER, this->new_state_size);

	this->hidden = new Layer(LEAKY_LAYER, this->hidden_size);
	this->hidden->input_layers.push_back(this->state_input);
	this->hidden->input_layers.push_back(this->new_state_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

ScoreNetwork::ScoreNetwork(int state_size,
						   int new_state_size,
						   int hidden_size) {
	this->state_size = state_size;
	this->new_state_size = new_state_size;
	this->hidden_size = hidden_size;

	construct();
}

ScoreNetwork::ScoreNetwork(ScoreNetwork* original) {
	this->state_size = original->state_size;
	this->new_state_size = 0;
	this->hidden_size = original->hidden_size;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

ScoreNetwork::ScoreNetwork(ifstream& input_file) {
	string state_size_line;
	getline(input_file, state_size_line);
	this->state_size = stoi(state_size_line);

	this->new_state_size = 0;

	string hidden_size_line;
	getline(input_file, hidden_size_line);
	this->hidden_size = stoi(hidden_size_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

ScoreNetwork::~ScoreNetwork() {
	delete this->state_input;
	delete this->new_state_input;
	delete this->hidden;
	delete this->output;
}

void ScoreNetwork::activate(vector<double>& state_vals) {
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void ScoreNetwork::activate(vector<double>& state_vals,
							ScoreNetworkHistory* history) {
	activate(state_vals);

	history->save_weights();
}

void ScoreNetwork::backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_errors[s_index] += this->state_input->errors[s_index];
		this->state_input->errors[s_index] = 0.0;
	}
}

void ScoreNetwork::backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& state_errors,
		vector<double>& state_vals_snapshot,
		ScoreNetworkHistory* history) {
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[s_index];
	}
	history->reset_weights();

	backprop_errors_with_no_weight_change(output_error,
										  state_errors);
}

void ScoreNetwork::backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop_weights_with_no_error_signal();

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->update_weights(hidden_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > target_max_update) {
				output_learning_rate = target_max_update/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void ScoreNetwork::backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update,
		vector<double>& state_vals_snapshot,
		ScoreNetworkHistory* history) {
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[s_index];
	}
	history->reset_weights();

	backprop_weights_with_no_error_signal(output_error,
										  target_max_update);
}

void ScoreNetwork::new_activate(vector<double>& state_vals,
								vector<double>& new_state_vals) {
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}
	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals[s_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void ScoreNetwork::new_activate(vector<double>& state_vals,
								vector<double>& new_state_vals,
								ScoreNetworkHistory* history) {
	new_activate(state_vals,
				 new_state_vals);

	history->save_weights();
}

void ScoreNetwork::new_backprop(double output_error,
								vector<double>& new_state_errors,
								double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		new_state_errors[s_index] += this->new_state_input->errors[s_index];
		this->new_state_input->errors[s_index] = 0.0;
	}

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->update_weights(hidden_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > target_max_update) {
				output_learning_rate = target_max_update/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void ScoreNetwork::new_backprop(double output_error,
								vector<double>& new_state_errors,
								double target_max_update,
								vector<double>& state_vals_snapshot,
								vector<double>& new_state_vals_snapshot,
								ScoreNetworkHistory* history) {
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[s_index];
	}
	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals_snapshot[s_index];
	}
	history->reset_weights();

	new_backprop(output_error,
				 new_state_errors,
				 target_max_update);
}

void ScoreNetwork::new_lasso_backprop(double output_error,
									  vector<double>& new_state_errors,
									  double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		new_state_errors[s_index] += this->new_state_input->errors[s_index];
		this->new_state_input->errors[s_index] = 0.0;
	}

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->lasso_update_weights(this->lasso_weights,
											   hidden_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > target_max_update) {
				output_learning_rate = target_max_update/output_max_update;
			}
			this->output->lasso_update_weights(DEFAULT_LASSO_WEIGHT,
											   output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void ScoreNetwork::new_lasso_backprop(double output_error,
									  vector<double>& new_state_errors,
									  double target_max_update,
									  vector<double>& state_vals_snapshot,
									  vector<double>& new_state_vals_snapshot,
									  ScoreNetworkHistory* history) {
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[s_index];
	}
	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals_snapshot[s_index];
	}
	history->reset_weights();

	new_lasso_backprop(output_error,
					   new_state_errors,
					   target_max_update);
}

void ScoreNetwork::update_lasso_weights(int new_furthest_distance) {
	this->lasso_weights = vector<vector<double>>(2);
	this->lasso_weights[0] = vector<double>(this->state_size, 0.0);
	this->lasso_weights[1] = vector<double>(NUM_NEW_STATES);
	for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
		this->lasso_weights[1][s_index] = new_furthest_distance*DEFAULT_NEW_STATE_LASSO_WEIGHTS[s_index];
	}
}

void ScoreNetwork::clean(int num_new_states) {
	int size_diff = this->new_state_size - num_new_states;
	for (int s_index = 0; s_index < size_diff; s_index++) {
		this->new_state_input->acti_vals.pop_back();
		this->new_state_input->errors.pop_back();

		for (int n_index = 0; n_index < (int)this->hidden->acti_vals.size(); n_index++) {
			this->hidden->weights[n_index][1].pop_back();
			this->hidden->weight_updates[n_index][1].pop_back();
		}
	}
	this->new_state_size = num_new_states;
}

void ScoreNetwork::finalize_new_state(int new_total_states) {
	this->hidden->score_hidden_finalize(new_total_states);

	int size_diff = new_total_states - this->state_size;
	for (int s_index = 0; s_index < size_diff; s_index++) {
		this->state_input->acti_vals.push_back(0.0);
		this->state_input->errors.push_back(0.0);
	}
	this->state_size = new_total_states;

	this->new_state_size = 0;

	this->new_state_input->acti_vals.clear();
	this->new_state_input->errors.clear();

	this->lasso_weights.clear();
}

void ScoreNetwork::add_state() {
	this->hidden->score_hidden_add_state();

	this->state_input->acti_vals.push_back(0.0);
	this->state_input->errors.push_back(0.0);

	this->state_size++;
}

void ScoreNetwork::save(ofstream& output_file) {
	output_file << this->state_size << endl;
	output_file << this->hidden_size << endl;

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

ScoreNetworkHistory::ScoreNetworkHistory(ScoreNetwork* network) {
	this->network = network;
}

void ScoreNetworkHistory::save_weights() {
	this->hidden_history.reserve(this->network->hidden_size);
	for (int n_index = 0; n_index < this->network->hidden_size; n_index++) {
		this->hidden_history.push_back(this->network->hidden->acti_vals[n_index]);
	}
}

void ScoreNetworkHistory::reset_weights() {
	for (int n_index = 0; n_index < this->network->hidden_size; n_index++) {
		this->network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
}
