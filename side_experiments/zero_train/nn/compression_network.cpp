#include "compression_network.h"

using namespace std;

void CompressionNetwork::construct() {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	this->scopes_on_input = new Layer(LINEAR_LAYER, (int)this->scope_sizes.size());

	int total_state_size = 0;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		total_state_size += this->scope_sizes[sc_index];
	}
	this->hidden = new Layer(RELU_LAYER, 30*total_state_size);
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[sc_index]);
	}
	this->hidden->input_layers.push_back(this->scopes_on_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, this->scope_sizes[this->scope_sizes.size()-2]);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();
}

CompressionNetwork::CompressionNetwork(vector<int> scope_sizes) {
	this->scope_sizes = scope_sizes;

	construct();
}

CompressionNetwork::CompressionNetwork(ifstream& input_file) {
	string num_scope_sizes_line;
	getline(input_file, num_scope_sizes_line);
	int num_scope_sizes = stoi(num_scope_sizes_line);
	this->scope_sizes.reserve(num_scope_sizes);
	for (int sc_index = 0; sc_index < num_scope_sizes; sc_index++) {
		string scope_size_line;
		getline(input_file, scope_size_line);
		this->scope_sizes.push_back(stoi(scope_size_line));
	}

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

CompressionNetwork::~CompressionNetwork() {
	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}
	delete this->scopes_on_input;

	delete this->hidden;
	delete this->output;
}

void CompressionNetwork::activate(vector<vector<double>>& state_vals,
								  vector<bool>& scopes_on) {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}

		if (scopes_on[sc_index]) {
			this->scopes_on_input->acti_vals[sc_index] = 1.0;
		} else {
			this->scopes_on_input->acti_vals[sc_index] = 0.0;
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void CompressionNetwork::backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop();
}

void CompressionNetwork::backprop_weights_with_no_error_signal(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop_weights_with_no_error_signal();
}

void CompressionNetwork::calc_max_update(double& max_update,
										 double learning_rate) {
	this->hidden->calc_max_update(max_update,
								  learning_rate);
	this->output->calc_max_update(max_update,
								  learning_rate);
}

void CompressionNetwork::update_weights(double factor,
										double learning_rate) {
	this->hidden->update_weights(factor,
								 learning_rate);
	this->output->update_weights(factor,
								 learning_rate);
}

void CompressionNetwork::save(ofstream& output_file) {
	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}
