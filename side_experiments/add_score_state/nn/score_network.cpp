#include "score_network.h"

using namespace std;

void ScoreNetwork::construct() {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	this->obs_input = new Layer(LINEAR_LAYER, 1);

	int total_state_size = 0;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		total_state_size += this->scope_sizes[sc_index];
	}
	this->hidden = new Layer(RELU_LAYER, 20*total_state_size);
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[sc_index]);
	}
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();
}

ScoreNetwork::ScoreNetwork(vector<int> scope_sizes) {
	this->scope_sizes = scope_sizes;

	construct();
}

ScoreNetwork::ScoreNetwork(ifstream& input_file) {
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

ScoreNetwork::ScoreNetwork(ScoreNetwork* original) {
	this->scope_sizes = original->scope_sizes;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

ScoreNetwork::~ScoreNetwork() {
	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}
	delete this->obs_input;

	delete this->hidden;
	delete this->output;
}

void ScoreNetwork::activate(vector<vector<double>>& state_vals,
							vector<double>& obs) {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	this->hidden->activate();
	this->output->activate();
}

void ScoreNetwork::backprop_weights_with_no_error_signal(double target_val) {
	this->output->errors[0] = target_val - this->output->acti_vals[0];

	this->output->backprop();
	this->hidden->backprop_weights_with_no_error_signal();
}

void ScoreNetwork::calc_max_update(double& max_update,
								   double learning_rate) {
	this->hidden->calc_max_update(max_update,
								  learning_rate);
	this->output->calc_max_update(max_update,
								  learning_rate);
}

void ScoreNetwork::update_weights(double factor,
								  double learning_rate) {
	this->hidden->update_weights(factor,
								 learning_rate);
	this->output->update_weights(factor,
								 learning_rate);
}

void ScoreNetwork::save(ofstream& output_file) {
	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}
