#include "fold_network.h"

#include <iostream>

using namespace std;

void FoldNetwork::construct() {
	for (int f_index = 0; f_index < (int)this->flat_sizes.size(); f_index++) {
		this->flat_inputs.push_back(new Layer(LINEAR_LAYER, this->flat_sizes[f_index]));
	}
	this->s_input_input = new Layer(LINEAR_LAYER, this->s_input_size);
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	this->hidden = new Layer(LEAKY_LAYER, this->hidden_size);
	for (int f_index = 0; f_index < (int)this->flat_sizes.size(); f_index++) {
		this->hidden->input_layers.push_back(this->flat_inputs[f_index]);
	}
	this->hidden->input_layers.push_back(this->s_input_input);
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[sc_index]);
	}
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, this->output_size);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

FoldNetwork::FoldNetwork(vector<int> flat_sizes,
						 int output_size,
						 int outer_s_input_size,
						 int outer_state_size,
						 int hidden_size) {
	this->flat_sizes = flat_sizes;
	this->output_size = output_size;
	
	this->s_input_size = outer_s_input_size;

	this->scope_sizes.push_back(outer_state_size);

	this->hidden_size = hidden_size;

	this->fold_index = -1;
	this->subfold_index = -1;

	construct();
}

FoldNetwork::FoldNetwork(int output_size,
						 int outer_s_input_size,
						 vector<int> scope_sizes,
						 int hidden_size) {
	this->output_size = output_size;

	this->s_input_size = outer_s_input_size;

	this->scope_sizes = scope_sizes;

	this->hidden_size = hidden_size;

	this->fold_index = -1;	// doesn't matter
	this->subfold_index = -1;

	construct();
}

FoldNetwork::FoldNetwork(FoldNetwork* original) {
	this->flat_sizes = original->flat_sizes;
	this->output_size = original->output_size;
	this->s_input_size = original->s_input_size;
	this->scope_sizes = original->scope_sizes;

	this->hidden_size = original->hidden_size;

	this->fold_index = original->fold_index;
	this->subfold_index = original->subfold_index;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

FoldNetwork::FoldNetwork(ifstream& input_file) {
	string num_flat_sizes_line;
	getline(input_file, num_flat_sizes_line);
	int num_flat_sizes = stoi(num_flat_sizes_line);
	for (int f_index = 0; f_index < num_flat_sizes; f_index++) {
		string flat_size_line;
		getline(input_file, flat_size_line);
		this->flat_sizes.push_back(stoi(flat_size_line));
	}

	string output_size_line;
	getline(input_file, output_size_line);
	this->output_size = stoi(output_size_line);

	string s_input_size_line;
	getline(input_file, s_input_size_line);
	this->s_input_size = stoi(s_input_size_line);

	string num_scope_sizes_line;
	getline(input_file, num_scope_sizes_line);
	int num_scope_sizes = stoi(num_scope_sizes_line);
	for (int sc_index = 0; sc_index < num_scope_sizes; sc_index++) {
		string scope_size_line;
		getline(input_file, scope_size_line);
		this->scope_sizes.push_back(stoi(scope_size_line));
	}

	string hidden_size_line;
	getline(input_file, hidden_size_line);
	this->hidden_size = stoi(hidden_size_line);

	string fold_index_line;
	getline(input_file, fold_index_line);
	this->fold_index = stoi(fold_index_line);

	string subfold_index_line;
	getline(input_file, subfold_index_line);
	this->subfold_index = stoi(subfold_index_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

FoldNetwork::~FoldNetwork() {
	for (int f_index = 0; f_index < (int)this->flat_sizes.size(); f_index++) {
		delete this->flat_inputs[f_index];
	}
	delete this->s_input_input;
	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}

	delete this->hidden;
	delete this->output;
}

void FoldNetwork::activate(vector<vector<double>>& flat_vals,
						   vector<double>& outer_s_input_vals,
						   vector<double>& outer_state_vals) {
	for (int f_index = 0; f_index < (int)this->flat_sizes.size(); f_index++) {
		for (int s_index = 0; s_index < this->flat_sizes[f_index]; s_index++) {
			this->flat_inputs[f_index]->acti_vals[s_index] = flat_vals[f_index][s_index];
		}
	}
	for (int s_index = 0; s_index < this->s_input_size; s_index++) {
		this->s_input_input->acti_vals[s_index] = outer_s_input_vals[s_index];
	}
	for (int s_index = 0; s_index < this->scope_sizes[0]; s_index++) {
		this->state_inputs[0]->acti_vals[s_index] = outer_state_vals[s_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldNetwork::activate(vector<vector<double>>& flat_vals,
						   vector<double>& outer_s_input_vals,
						   vector<double>& outer_state_vals,
						   FoldNetworkHistory* history) {
	activate(flat_vals,
			 outer_s_input_vals,
			 outer_state_vals);

	history->save_weights();
}

void FoldNetwork::backprop(vector<double>& errors,
						   double target_max_update) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop();

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

void FoldNetwork::backprop(vector<double>& errors,
						   double target_max_update,
						   FoldNetworkHistory* history) {
	history->reset_weights();

	backprop(errors,
			 target_max_update);
}

void FoldNetwork::backprop_errors_with_no_weight_change(vector<double>& errors) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();
}

void FoldNetwork::backprop_errors_with_no_weight_change(vector<double>& errors,
														FoldNetworkHistory* history) {
	history->reset_weights();

	backprop_errors_with_no_weight_change(errors);
}

void FoldNetwork::backprop_weights_with_no_error_signal(vector<double>& errors,
														double target_max_update) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

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

void FoldNetwork::backprop_weights_with_no_error_signal(vector<double>& errors,
														double target_max_update,
														FoldNetworkHistory* history) {
	history->reset_weights();

	backprop_weights_with_no_error_signal(errors,
										  target_max_update);
}

void FoldNetwork::save(ofstream& output_file) {
	output_file << this->flat_sizes.size() << endl;
	for (int f_index = 0; f_index < (int)this->flat_sizes.size(); f_index++) {
		output_file << this->flat_sizes[f_index] << endl;
	}

	output_file << this->output_size << endl;

	output_file << this->s_input_size << endl;

	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	output_file << this->hidden_size << endl;

	output_file << this->fold_index << endl;
	output_file << this->subfold_index << endl;

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

FoldNetworkHistory::FoldNetworkHistory(FoldNetwork* network) {
	this->network = network;
}

void FoldNetworkHistory::save_weights() {
	this->flat_input_histories = vector<vector<double>>(this->network->flat_sizes.size());
	for (int f_index = this->network->fold_index+1; f_index < (int)this->network->flat_sizes.size(); f_index++) {
		this->flat_input_histories[f_index].reserve(this->network->flat_sizes[f_index]);
		for (int n_index = 0; n_index < this->network->flat_sizes[f_index]; n_index++) {
			this->flat_input_histories[f_index].push_back(this->network->flat_inputs[f_index]->acti_vals[n_index]);
		}
	}

	this->s_input_input_history.reserve(this->network->s_input_size);
	for (int n_index = 0; n_index < this->network->s_input_size; n_index++) {
		this->s_input_input_history.push_back(this->network->s_input_input->acti_vals[n_index]);
	}

	this->state_inputs_historys = vector<vector<double>>(this->network->scope_sizes.size());
	for (int sc_index = this->network->subfold_index+1; sc_index < (int)this->network->scope_sizes.size(); sc_index++) {
		this->state_inputs_historys[sc_index].reserve(this->network->scope_sizes[sc_index]);
		for (int st_index = 0; st_index < this->network->scope_sizes[sc_index]; st_index++) {
			this->state_inputs_historys[sc_index].push_back(this->network->state_inputs[sc_index]->acti_vals[st_index]);
		}
	}

	this->hidden_history.reserve(this->network->hidden_size);
	for (int n_index = 0; n_index < this->network->hidden_size; n_index++) {
		this->hidden_history.push_back(network->hidden->acti_vals[n_index]);
	}
}

void FoldNetworkHistory::reset_weights() {
	for (int f_index = this->network->fold_index+1; f_index < (int)this->network->flat_sizes.size(); f_index++) {
		for (int n_index = 0; n_index < this->network->flat_sizes[f_index]; n_index++) {
			this->network->flat_inputs[f_index]->acti_vals[n_index] = this->flat_input_histories[f_index][n_index];
		}
	}

	for (int n_index = 0; n_index < this->network->s_input_size; n_index++) {
		this->network->s_input_input->acti_vals[n_index] = this->s_input_input_history[n_index];
	}

	for (int sc_index = this->network->subfold_index+1; sc_index < (int)this->network->scope_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->network->scope_sizes[sc_index]; st_index++) {
			this->network->state_inputs[sc_index]->acti_vals[st_index] = this->state_inputs_historys[sc_index][st_index];
		}
	}

	for (int n_index = 0; n_index < this->network->hidden_size; n_index++) {
		this->network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
}
