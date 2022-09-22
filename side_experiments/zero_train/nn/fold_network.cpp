#include "fold_network.h"

#include <iostream>

using namespace std;

void FoldNetwork::construct() {
	this->flat_input = new Layer(LINEAR_LAYER, this->flat_size);
	this->activated_input = new Layer(LINEAR_LAYER, this->flat_size);
	this->obs_input = new Layer(LINEAR_LAYER, 1);

	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	int total_input_size = 2*this->flat_size + 1;
	this->hidden = new Layer(LEAKY_LAYER, 2*total_input_size*total_input_size);
	this->hidden->input_layers.push_back(this->flat_input);
	this->hidden->input_layers.push_back(this->activated_input);
	this->hidden->input_layers.push_back(this->obs_input);
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[sc_index]);
	}
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, this->output_size);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();
}

FoldNetwork::FoldNetwork(int flat_size,
						 int output_size) {
	this->flat_size = flat_size;
	this->output_size = output_size;

	this->fold_index = -1;

	construct();
}

FoldNetwork::FoldNetwork(ifstream& input_file) {
	string flat_size_line;
	getline(input_file, flat_size_line);
	this->flat_size = stoi(flat_size_line);

	string output_size_line;
	getline(input_file, output_size_line);
	this->output_size = stoi(output_size_line);

	string fold_index_line;
	getline(input_file, fold_index_line);
	this->fold_index = stoi(fold_index_line);

	string num_scope_sizes_line;
	getline(input_file, num_scope_sizes_line);
	int num_scope_sizes = stoi(num_scope_sizes_line);
	for (int sc_index = 0; sc_index < num_scope_sizes; sc_index++) {
		string scope_size_line;
		getline(input_file, scope_size_line);
		this->scope_sizes.push_back(stoi(scope_size_line));
	}

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

FoldNetwork::FoldNetwork(FoldNetwork* original) {
	this->flat_size = original->flat_size;
	this->output_size = original->output_size;

	this->fold_index = original->fold_index;

	this->scope_sizes = original->scope_sizes;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

FoldNetwork::~FoldNetwork() {
	delete this->flat_input;
	delete this->activated_input;
	delete this->obs_input;
	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}

	delete this->hidden;
	delete this->output;
}

void FoldNetwork::activate(double* flat_inputs,
						   bool* activated,
						   vector<double>& obs) {
	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		this->flat_input->acti_vals[f_index] = flat_inputs[f_index];

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	this->hidden->activate();
	this->output->activate();
}

void FoldNetwork::backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop();
}

void FoldNetwork::calc_max_update(double& max_update,
								  double learning_rate) {
	this->hidden->calc_max_update(max_update,
								  learning_rate);
	this->output->calc_max_update(max_update,
								  learning_rate);
}

void FoldNetwork::update_weights(double factor,
								 double learning_rate) {
	this->hidden->update_weights(factor,
								 learning_rate);
	this->output->update_weights(factor,
								 learning_rate);
}

void FoldNetwork::add_scope(int scope_size) {
	this->scope_sizes.push_back(scope_size);
	this->state_inputs.push_back(new Layer(LINEAR_LAYER, scope_size));
	this->hidden->fold_add_scope(this->state_inputs.back());
}

void FoldNetwork::pop_scope() {
	this->scope_sizes.pop_back();
	delete this->state_inputs.back();
	this->state_inputs.pop_back();
	this->hidden->fold_pop_scope();
}

void FoldNetwork::reset_last() {
	this->hidden->fold_pop_scope();
	this->hidden->fold_add_scope(this->state_inputs.back());
}

void FoldNetwork::activate(double* flat_inputs,
						   bool* activated,
						   vector<double>& obs,
						   vector<vector<double>>& state_vals) {
	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		if (this->fold_index >= f_index) {
			this->flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->flat_input->acti_vals[f_index] = flat_inputs[f_index];
		}

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldNetwork::backprop_last_state(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->fold_backprop_last_state();
}

void FoldNetwork::calc_max_update_last_state(double& max_update,
											 double learning_rate) {
	this->output->calc_max_update(max_update,
								  learning_rate);
	this->hidden->fold_calc_max_update_last_state(max_update,
												  learning_rate);
}

void FoldNetwork::update_weights_last_state(double factor,
											double learning_rate) {
	this->output->update_weights(factor,
								 learning_rate);
	this->hidden->fold_update_weights_last_state(factor,
												 learning_rate);
}

void FoldNetwork::backprop_full_state(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->fold_backprop_full_state();
}

void FoldNetwork::calc_max_update_full_state(double& max_update,
											 double learning_rate) {
	this->output->calc_max_update(max_update,
								  learning_rate);
	this->hidden->fold_calc_max_update_full_state(max_update,
												  learning_rate);
}

void FoldNetwork::update_weights_full_state(double factor,
											double learning_rate) {
	this->output->update_weights(factor,
								 learning_rate);
	this->hidden->fold_update_weights_full_state(factor,
												 learning_rate);
}

void FoldNetwork::save(ofstream& output_file) {
	output_file << this->flat_size << endl;
	output_file << this->output_size << endl;

	output_file << this->fold_index << endl;

	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}
