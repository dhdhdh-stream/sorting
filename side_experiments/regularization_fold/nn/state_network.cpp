#include "state_network.h"

#include <iostream>

#include "abstract_experiment.h"
#include "layer.h"

using namespace std;

void StateNetwork::construct() {
	this->obs_input = new Layer(LINEAR_LAYER, 1);
	this->state_input = new Layer(LINEAR_LAYER, (int)this->state_indexes.size());
	this->new_state_input = new Layer(LINEAR_LAYER, (int)this->new_state_indexes.size());
	this->new_input_input = new Layer(LINEAR_LAYER, this->new_input_size);

	this->hidden = new Layer(LEAKY_LAYER, this->hidden_size);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->input_layers.push_back(this->state_input);
	this->hidden->input_layers.push_back(this->new_state_input);
	this->hidden->input_layers.push_back(this->new_input_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

StateNetwork::StateNetwork(int state_size,
						   int new_state_size,
						   int new_input_size,
						   int hidden_size) {
	for (int s_index = 0; s_index < state_size; s_index++) {
		this->state_indexes.push_back(s_index);
	}
	for (int s_index = 0; s_index < new_state_size; s_index++) {
		this->new_state_indexes.push_back(s_index);
	}
	this->new_input_size = new_input_size;
	this->hidden_size = hidden_size;

	construct();
}

StateNetwork::StateNetwork(StateNetwork* original) {
	this->state_indexes = original->state_indexes;
	this->new_input_size = 0;
	this->hidden_size = original->hidden_size;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

StateNetwork::StateNetwork(ifstream& input_file) {
	string state_size_line;
	getline(input_file, state_size_line);
	int state_size = stoi(state_size_line);
	for (int s_index = 0; s_index < state_size; s_index++) {
		string state_index_line;
		getline(input_file, state_index_line);
		this->state_indexes.push_back(stoi(state_index_line));
	}

	this->new_input_size = 0;

	string hidden_size_line;
	getline(input_file, hidden_size_line);
	this->hidden_size = stoi(hidden_size_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

StateNetwork::~StateNetwork() {
	delete this->obs_input;
	delete this->state_input;
	delete this->new_state_input;
	delete this->new_input_input;
	delete this->hidden;
	delete this->output;
}

void StateNetwork::activate(double obs_val,
							vector<double>& state_vals) {
	this->obs_input->acti_vals[0] = obs_val;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[this->state_indexes[s_index]];
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::activate(double obs_val,
							vector<double>& state_vals,
							StateNetworkHistory* history) {
	activate(obs_val,
			 state_vals);

	history->save_weights();
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();

	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		state_errors[this->state_indexes[s_index]] += this->state_input->errors[s_index];
		this->state_input->errors[s_index] = 0.0;
	}
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& state_errors,
		double obs_snapshot,
		vector<double>& state_vals_snapshot,
		StateNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_snapshot;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[this->state_indexes[s_index]];
	}
	history->reset_weights();

	backprop_errors_with_no_weight_change(output_error,
										  state_errors);
}

void StateNetwork::new_activate(double obs_val,
								vector<double>& state_vals,
								vector<double>& new_state_vals) {
	this->obs_input->acti_vals[0] = obs_val;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[this->state_indexes[s_index]];
	}
	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals[this->new_state_indexes[s_index]];
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::new_activate(double obs_val,
								vector<double>& state_vals,
								vector<double>& new_state_vals,
								StateNetworkHistory* history) {
	new_activate(obs_val,
				 state_vals,
				 new_state_vals);

	history->save_weights();
}

void StateNetwork::new_backprop(double output_error,
								vector<double>& new_state_errors,
								double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		new_state_errors[this->new_state_indexes[s_index]] += this->new_state_input->errors[s_index];
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

void StateNetwork::new_backprop(double output_error,
								vector<double>& new_state_errors,
								double target_max_update,
								double obs_snapshot,
								vector<double>& state_vals_snapshot,
								vector<double>& new_state_vals_snapshot,
								StateNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_snapshot;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[this->state_indexes[s_index]];
	}
	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals_snapshot[this->new_state_indexes[s_index]];
	}
	history->reset_weights();

	new_backprop(output_error,
				 new_state_errors,
				 target_max_update);
}

void StateNetwork::new_scaled_backprop(double output_error,
									   vector<double>& new_state_errors,
									   double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		// all new states still present
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
			this->hidden->state_hidden_scaled_update_weights(hidden_learning_rate);
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

void StateNetwork::new_scaled_backprop(double output_error,
									   vector<double>& new_state_errors,
									   double target_max_update,
									   double obs_snapshot,
									   vector<double>& state_vals_snapshot,
									   vector<double>& new_state_vals_snapshot,
									   StateNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_snapshot;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[this->state_indexes[s_index]];
	}
	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals_snapshot[this->new_state_indexes[s_index]];
	}
	history->reset_weights();

	new_scaled_backprop(output_error,
						new_state_errors,
						target_max_update);
}

void StateNetwork::new_lasso_backprop(double output_error,
									  vector<double>& new_state_errors,
									  double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		// all new states still present
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

void StateNetwork::new_lasso_backprop(double output_error,
									  vector<double>& new_state_errors,
									  double target_max_update,
									  double obs_snapshot,
									  vector<double>& state_vals_snapshot,
									  vector<double>& new_state_vals_snapshot,
									  StateNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_snapshot;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[this->state_indexes[s_index]];
	}
	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals_snapshot[this->new_state_indexes[s_index]];
	}
	history->reset_weights();

	new_lasso_backprop(output_error,
					   new_state_errors,
					   target_max_update);
}

void StateNetwork::new_activate(double obs_val,
								vector<double>& state_vals,
								vector<double>& new_state_vals,
								double new_input_val) {
	this->obs_input->acti_vals[0] = obs_val;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[this->state_indexes[s_index]];
	}
	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals[this->new_state_indexes[s_index]];
	}
	this->new_input_input->acti_vals[0] = new_input_val;

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::new_activate(double obs_val,
								vector<double>& state_vals,
								vector<double>& new_state_vals,
								double new_input_val,
								StateNetworkHistory* history) {
	new_activate(obs_val,
				 state_vals,
				 new_state_vals,
				 new_input_val);

	history->save_weights();
}

void StateNetwork::new_backprop(double output_error,
								vector<double>& new_state_errors,
								double& new_input_error,
								double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		new_state_errors[this->new_state_indexes[s_index]] += this->new_state_input->errors[s_index];
		this->new_state_input->errors[s_index] = 0.0;
	}

	new_input_error += this->new_input_input->errors[0];
	this->new_input_input->errors[0] = 0.0;

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

void StateNetwork::new_backprop(double output_error,
								vector<double>& new_state_errors,
								double& new_input_error,
								double target_max_update,
								double obs_snapshot,
								vector<double>& state_vals_snapshot,
								vector<double>& new_state_vals_snapshot,
								double new_input_snapshot,
								StateNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_snapshot;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[this->state_indexes[s_index]];
	}
	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals_snapshot[this->new_state_indexes[s_index]];
	}
	this->new_input_input->acti_vals[0] = new_input_snapshot;
	history->reset_weights();

	new_backprop(output_error,
				 new_state_errors,
				 new_input_error,
				 target_max_update);
}

void StateNetwork::new_scaled_backprop(double output_error,
									   vector<double>& new_state_errors,
									   double& new_input_error,
									   double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		new_state_errors[this->new_state_indexes[s_index]] += this->new_state_input->errors[s_index];
		this->new_state_input->errors[s_index] = 0.0;
	}

	new_input_error += this->new_input_input->errors[0];
	this->new_input_input->errors[0] = 0.0;

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
			this->hidden->state_hidden_scaled_update_weights(hidden_learning_rate);
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

void StateNetwork::new_scaled_backprop(double output_error,
									   vector<double>& new_state_errors,
									   double& new_input_error,
									   double target_max_update,
									   double obs_snapshot,
									   vector<double>& state_vals_snapshot,
									   vector<double>& new_state_vals_snapshot,
									   double new_input_snapshot,
									   StateNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_snapshot;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[this->state_indexes[s_index]];
	}
	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals_snapshot[this->new_state_indexes[s_index]];
	}
	this->new_input_input->acti_vals[0] = new_input_snapshot;
	history->reset_weights();

	new_scaled_backprop(output_error,
						new_state_errors,
						new_input_error,
						target_max_update);
}

void StateNetwork::new_lasso_backprop(double output_error,
									  vector<double>& new_state_errors,
									  double& new_input_error,
									  double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		new_state_errors[this->new_state_indexes[s_index]] += this->new_state_input->errors[s_index];
		this->new_state_input->errors[s_index] = 0.0;
	}

	new_input_error += this->new_input_input->errors[0];
	this->new_input_input->errors[0] = 0.0;

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

void StateNetwork::new_lasso_backprop(double output_error,
									  vector<double>& new_state_errors,
									  double& new_input_error,
									  double target_max_update,
									  double obs_snapshot,
									  vector<double>& state_vals_snapshot,
									  vector<double>& new_state_vals_snapshot,
									  double new_input_snapshot,
									  StateNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_snapshot;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals_snapshot[this->state_indexes[s_index]];
	}
	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals_snapshot[this->new_state_indexes[s_index]];
	}
	this->new_input_input->acti_vals[0] = new_input_snapshot;
	history->reset_weights();

	new_lasso_backprop(output_error,
					   new_state_errors,
					   new_input_error,
					   target_max_update);
}

void StateNetwork::update_lasso_weights(int new_furthest_distance) {
	this->lasso_weights = vector<vector<double>>(4);
	this->lasso_weights[0] = vector<double>{new_furthest_distance*DEFAULT_LASSO_WEIGHT};
	this->lasso_weights[1] = vector<double>(this->state_indexes.size(), 0.0);
	this->lasso_weights[2] = vector<double>(NUM_NEW_STATES);
	for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
		this->lasso_weights[2][s_index] = new_furthest_distance*DEFAULT_NEW_STATE_LASSO_WEIGHTS[s_index];
	}
	this->lasso_weights[3] = vector<double>(this->new_input_size, new_furthest_distance*DEFAULT_LASSO_WEIGHT);
}

void StateNetwork::clean(int num_new_states) {
	// remove global unnecessary
	int size_diff = (int)this->new_state_indexes.size() - num_new_states;
	for (int s_index = 0; s_index < size_diff; s_index++) {
		this->new_state_indexes.pop_back();

		this->new_state_input->acti_vals.pop_back();
		this->new_state_input->errors.pop_back();

		for (int n_index = 0; n_index < (int)this->hidden->acti_vals.size(); n_index++) {
			this->hidden->weights[n_index][2].pop_back();
			this->hidden->weight_updates[n_index][2].pop_back();
		}
	}

	// clean output layer
	// remove back to front
	for (int n_index = this->hidden_size-1; n_index >= 0; n_index--) {
		if (abs(this->output->weights[0][0][n_index]) < 0.05) {
			this->hidden->acti_vals.erase(this->hidden->acti_vals.begin()+n_index);
			this->hidden->errors.erase(this->hidden->errors.begin()+n_index);
			this->hidden->weights.erase(this->hidden->weights.begin()+n_index);
			this->hidden->constants.erase(this->hidden->constants.begin()+n_index);
			this->hidden->weight_updates.erase(this->hidden->weight_updates.begin()+n_index);
			this->hidden->constant_updates.erase(this->hidden->constant_updates.begin()+n_index);

			this->output->weights[0][0].erase(this->output->weights[0][0].begin()+n_index);
			this->output->weight_updates[0][0].erase(this->output->weight_updates[0][0].begin()+n_index);

			this->hidden_size--;
		}
	}

	// clean hidden layer
	// remove back to front
	for (int s_index = (int)this->state_indexes.size()-1; s_index >= 0; s_index--) {
		double sum_state_impact = 0.0;
		for (int n_index = 0; n_index < this->hidden_size; n_index++) {
			sum_state_impact += abs(this->hidden->weights[n_index][1][s_index]);
		}

		if (sum_state_impact < 0.1) {
			this->state_indexes.erase(this->state_indexes.begin()+s_index);
			this->state_input->acti_vals.erase(this->state_input->acti_vals.begin()+s_index);
			this->state_input->errors.erase(this->state_input->errors.begin()+s_index);

			for (int n_index = 0; n_index < this->hidden_size; n_index++) {
				this->hidden->weights[n_index][1].erase(this->hidden->weights[n_index][1].begin()+s_index);
				this->hidden->weight_updates[n_index][1].erase(this->hidden->weight_updates[n_index][1].begin()+s_index);
			}
		}
	}
	for (int s_index = (int)this->new_state_indexes.size()-1; s_index >= 0; s_index--) {
		double sum_state_impact = 0.0;
		for (int n_index = 0; n_index < this->hidden_size; n_index++) {
			sum_state_impact += abs(this->hidden->weights[n_index][2][s_index]);
		}

		if (sum_state_impact < 0.1) {
			this->new_state_indexes.erase(this->new_state_indexes.begin()+s_index);
			this->new_state_input->acti_vals.erase(this->new_state_input->acti_vals.begin()+s_index);
			this->new_state_input->errors.erase(this->new_state_input->errors.begin()+s_index);

			for (int n_index = 0; n_index < this->hidden_size; n_index++) {
				this->hidden->weights[n_index][2].erase(this->hidden->weights[n_index][2].begin()+s_index);
				this->hidden->weight_updates[n_index][2].erase(this->hidden->weight_updates[n_index][2].begin()+s_index);
			}
		}
	}
	if (this->new_input_size > 0) {
		double sum_state_impact = 0.0;
		for (int n_index = 0; n_index < this->hidden_size; n_index++) {
			sum_state_impact += abs(this->hidden->weights[n_index][3][0]);
		}

		if (sum_state_impact < 0.1) {
			this->new_input_size = 0;
			this->new_input_input->acti_vals.clear();
			this->new_input_input->errors.clear();

			for (int n_index = 0; n_index < this->hidden_size; n_index++) {
				this->hidden->weights[n_index][3].clear();
				this->hidden->weight_updates[n_index][3].clear();
			}
		}
	}

	this->lasso_weights.clear();
}

void StateNetwork::finalize_new_state(int new_state_index,
									  int new_index) {
	int layer_index = -1;
	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		if (this->new_state_indexes[s_index] == new_state_index) {
			layer_index = s_index;
			break;
		}
	}

	if (layer_index != -1) {
		this->hidden->state_hidden_finalize_new_state(layer_index);

		this->state_indexes.push_back(new_index);

		this->state_input->acti_vals.push_back(0.0);
		this->state_input->errors.push_back(0.0);

		this->new_state_indexes.erase(this->new_state_indexes.begin()+layer_index);
		this->new_state_input->acti_vals.erase(this->new_state_input->acti_vals.begin()+layer_index);
		this->new_state_input->errors.erase(this->new_state_input->errors.begin()+layer_index);
	}
}

void StateNetwork::finalize_new_input(int new_index) {
	if (this->new_input_size > 0) {
		this->hidden->state_hidden_finalize_new_input();

		this->state_indexes.push_back(new_index);

		this->state_input->acti_vals.push_back(0.0);
		this->state_input->errors.push_back(0.0);

		this->new_input_size = 0;
		this->new_input_input->acti_vals.clear();
		this->new_input_input->errors.clear();
	}
}

void StateNetwork::save(ofstream& output_file) {
	output_file << this->state_indexes.size() << endl;
	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		output_file << this->state_indexes[s_index] << endl;
	}

	output_file << this->hidden_size << endl;

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

StateNetworkHistory::StateNetworkHistory(StateNetwork* network) {
	this->network = network;
}

void StateNetworkHistory::save_weights() {
	this->hidden_history.reserve(this->network->hidden_size);
	for (int n_index = 0; n_index < this->network->hidden_size; n_index++) {
		this->hidden_history.push_back(this->network->hidden->acti_vals[n_index]);
	}
}

void StateNetworkHistory::reset_weights() {
	for (int n_index = 0; n_index < this->network->hidden_size; n_index++) {
		this->network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
}
