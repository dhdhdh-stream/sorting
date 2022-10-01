#include "score_network.h"

using namespace std;

void ScoreNetwork::construct() {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	this->obs_input = new Layer(LINEAR_LAYER, this->obs_size);

	int sum_size = 0;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		sum_size += this->scope_sizes[sc_index];
	}
	sum_size += this->obs_size;
	this->hidden = new Layer(RELU_LAYER, 10*sum_size);
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[sc_index]);
	}
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

ScoreNetwork::ScoreNetwork(vector<int> scope_sizes,
						   int obs_size) {
	this->scope_sizes = scope_sizes;
	this->obs_size = obs_size;

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

	string obs_size_line;
	getline(input_file, obs_size_line);
	this->obs_size = stoi(obs_size_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

ScoreNetwork::ScoreNetwork(ScoreNetwork* original) {
	this->scope_sizes = original->scope_sizes;
	this->obs_size = original->obs_size;

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

	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = obs[o_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void ScoreNetwork::activate(vector<vector<double>>& state_vals,
							vector<double>& obs,
							vector<AbstractNetworkHistory*>& network_historys) {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}

	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = obs[o_index];
	}

	this->hidden->activate();
	this->output->activate();

	ScoreNetworkHistory* network_history = new ScoreNetworkHistory(this);
	network_historys.push_back(network_history);
}

void ScoreNetwork::backprop(double target_val,
							double target_max_update) {
	this->output->errors[0] = target_val - this->output->acti_vals[0];

	this->output->backprop();
	this->hidden->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == 100) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
		if (hidden_learning_rate*hidden_max_update > target_max_update) {
			hidden_learning_rate = target_max_update/hidden_max_update;
		}
		this->hidden->update_weights(hidden_learning_rate);

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
		if (output_learning_rate*output_max_update > target_max_update) {
			output_learning_rate = target_max_update/output_max_update;
		}
		this->output->update_weights(output_learning_rate);

		this->epoch_iter = 0;
	}
}

void ScoreNetwork::backprop_weights_with_no_error_signal(
		double target_val,
		double target_max_update) {
	this->output->errors[0] = target_val - this->output->acti_vals[0];

	this->output->backprop();
	this->hidden->backprop_weights_with_no_error_signal();

	this->epoch_iter++;
	if (this->epoch_iter == 100) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
		if (hidden_learning_rate*hidden_max_update > target_max_update) {
			hidden_learning_rate = target_max_update/hidden_max_update;
		}
		this->hidden->update_weights(hidden_learning_rate);

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
		if (output_learning_rate*output_max_update > target_max_update) {
			output_learning_rate = target_max_update/output_max_update;
		}
		this->output->update_weights(output_learning_rate);

		this->epoch_iter = 0;
	}
}

void ScoreNetwork::save(ofstream& output_file) {
	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	output_file << this->obs_size << endl;

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

ScoreNetworkHistory::ScoreNetworkHistory(ScoreNetwork* network) {
	this->network = network;

	this->state_inputs_historys.reserve(network->state_inputs.size());
	for (int sc_index = 0; sc_index < (int)network->state_inputs.size(); sc_index++) {
		this->state_inputs_historys.push_back(vector<double>(network->state_inputs[sc_index]->acti_vals.size()));
		for (int st_index = 0; st_index < (int)network->state_inputs[sc_index]->acti_vals.size(); st_index++) {
			this->state_inputs_historys[sc_index][st_index] = network->state_inputs[sc_index]->acti_vals[st_index];
		}
	}
	this->obs_input_history.reserve(network->obs_input->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->obs_input->acti_vals.size(); n_index++) {
		this->obs_input_history.push_back(network->obs_input->acti_vals[n_index]);
	}

	this->hidden_history.reserve(network->hidden->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		this->hidden_history.push_back(network->hidden->acti_vals[n_index]);
	}
	this->output_history.reserve(network->output->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		this->output_history.push_back(network->output->acti_vals[n_index]);
	}
}

void ScoreNetworkHistory::reset_weights() {
	ScoreNetwork* network = (ScoreNetwork*)this->network;

	for (int sc_index = 0; sc_index < (int)network->state_inputs.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)network->state_inputs[sc_index]->acti_vals.size(); st_index++) {
			network->state_inputs[sc_index]->acti_vals[st_index] = this->state_inputs_historys[sc_index][st_index];
		}
	}
	for (int n_index = 0; n_index < (int)network->obs_input->acti_vals.size(); n_index++) {
		network->obs_input->acti_vals[n_index] = this->obs_input_history[n_index];
	}

	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		network->output->acti_vals[n_index] = this->output_history[n_index];
	}
}
