#include "score_network.h"

using namespace std;

ScoreNetwork::ScoreNetwork(vector<int> local_state_sizes) {
	this->local_state_sizes = local_state_sizes;

	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->local_state_sizes[l_index]));
	}

	this->obs_input = new Layer(LINEAR_LAYER, 1);

	int num_inputs = 1;
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		num_inputs += this->local_state_sizes[l_index];
	}

	this->hidden = new Layer(RELU_LAYER, 4*num_inputs);
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[l_index]);
	}
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch = 0;
	this->iter = 0;
}

ScoreNetwork::ScoreNetwork(ifstream& input_file) {
	string num_local_state_sizes_line;
	getline(input_file, num_local_state_sizes_line);
	int num_local_state_sizes = stoi(num_local_state_sizes_line);
	for (int l_index = 0; l_index < num_local_state_sizes; l_index++) {
		string state_size_line;
		getline(input_file, state_size_line);
		this->local_state_sizes.push_back(stoi(state_size_line));
	}

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	string epoch_line;
	getline(input_file, epoch_line);
	this->epoch = stoi(epoch_line);
	this->iter = 0;
}

ScoreNetwork::~ScoreNetwork() {
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		delete this->state_inputs[l_index];
	}
	delete this->obs_input;

	delete this->hidden;
	delete this->output;
}

void ScoreNetwork::insert_scope(int layer,
								int new_state_size) {
	this->local_state_sizes.insert(this->local_state_sizes.begin() + layer, new_state_size);

	this->state_inputs.insert(this->state_inputs.begin() + layer,
		new Layer(LINEAR_LAYER, new_state_size));

	this->hidden->insert_input_layer(layer, this->state_inputs[layer]);
	this->hidden->add_nodes(4*new_state_size);
	this->output->output_input_extend(4*new_state_size);
}

void ScoreNetwork::activate(vector<vector<double>>& state_vals,
							vector<double>& obs,
							vector<AbstractNetworkHistory*>& network_historys) {
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
			this->state_inputs[l_index]->acti_vals[s_index] = state_vals[l_index][s_index];
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	this->hidden->activate();
	this->output->activate();

	ScoreNetworkHistory* network_history = new ScoreNetworkHistory(this);
	network_historys.push_back(network_history);
}

void ScoreNetwork::backprop(vector<double>& errors) {
	this->output->errors[0] = errors[0];

	this->output->backprop();
	this->hidden->backprop();

	if (this->iter == 100) {
		double max_update = 0.0;
		calc_max_update(max_update,
						0.001,
						0.2);
		double factor = 1.0;
		if (max_update > 0.01) {
			factor = 0.01/max_update;
		}
		update_weights(factor,
					   0.001,
					   0.2);

		this->epoch++;
		this->iter = 0;
	} else {
		this->iter++;
	}
}

void ScoreNetwork::calc_max_update(double& max_update,
								   double learning_rate,
								   double momentum) {
	this->hidden->calc_max_update(max_update,
								  learning_rate,
								  momentum);
	this->output->calc_max_update(max_update,
								  learning_rate,
								  momentum);
}

void ScoreNetwork::update_weights(double factor,
								  double learning_rate,
								  double momentum) {
	this->hidden->update_weights(factor,
								 learning_rate,
								 momentum);
	this->output->update_weights(factor,
								 learning_rate,
								 momentum);
}

void ScoreNetwork::save(ofstream& output_file) {
	output_file << this->local_state_sizes.size() << endl;
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		output_file << this->local_state_sizes[l_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
	output_file << this->epoch << endl;
}

ScoreNetworkHistory::ScoreNetworkHistory(ScoreNetwork* network) {
	this->network = network;

	for (int l_index = 0; l_index < (int)network->local_state_sizes.size(); l_index++) {
		vector<double> layer_history;
		for (int s_index = 0; s_index < network->local_state_sizes[l_index]; s_index++) {
			layer_history.push_back(network->state_inputs[l_index]->acti_vals[s_index]);
		}
		this->state_inputs_history.push_back(layer_history);
	}

	obs_input_history.push_back(network->obs_input->acti_vals[0]);

	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		this->hidden_history.push_back(network->hidden->acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		this->output_history.push_back(network->output->acti_vals[n_index]);
	}
}

void ScoreNetworkHistory::reset_weights() {
	ScoreNetwork* network = (ScoreNetwork*)this->network;

	for (int l_index = 0; l_index < (int)network->local_state_sizes.size(); l_index++) {
		for (int s_index = 0; s_index < network->local_state_sizes[l_index]; s_index++) {
			network->state_inputs[l_index]->acti_vals[s_index] = this->state_inputs_history[l_index][s_index];
		}
	}

	network->obs_input->acti_vals[0] = this->obs_input_history[0];

	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		network->output->acti_vals[n_index] = this->output_history[n_index];
	}
}
