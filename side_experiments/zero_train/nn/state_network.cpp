#include "state_network.h"

using namespace std;

void StateNetwork::construct() {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	this->scopes_on_input = new Layer(LINEAR_LAYER, (int)this->scope_sizes.size());

	this->obs_input = new Layer(LINEAR_LAYER, 1);

	int total_state_size = 0;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		total_state_size += this->scope_sizes[sc_index];
	}
	this->hidden = new Layer(RELU_LAYER, 30*total_state_size);
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[sc_index]);
	}
	this->hidden->input_layers.push_back(this->scopes_on_input);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, this->scope_sizes.back());
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();
}

StateNetwork::StateNetwork(vector<int> scope_sizes) {
	this->scope_sizes = scope_sizes;

	construct();
}

StateNetwork::StateNetwork(ifstream& input_file) {
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

StateNetwork::~StateNetwork() {
	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}
	delete this->scopes_on_input;
	delete this->obs_input;

	delete this->hidden;
	delete this->output;
}

void StateNetwork::activate(vector<vector<double>>& state_vals,
							vector<bool>& scopes_on,
							vector<double>& obs) {
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

	this->obs_input->acti_vals[0] = obs[0];

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop();
}

void StateNetwork::backprop_weights_with_no_error_signal(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop_weights_with_no_error_signal();
}

void StateNetwork::calc_max_update(double& max_update,
								   double learning_rate) {
	this->hidden->calc_max_update(max_update,
								  learning_rate);
	this->output->calc_max_update(max_update,
								  learning_rate);
}

void StateNetwork::update_weights(double factor,
								  double learning_rate) {
	this->hidden->update_weights(factor,
								 learning_rate);
	this->output->update_weights(factor,
								 learning_rate);
}

void StateNetwork::save(ofstream& output_file) {
	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}
