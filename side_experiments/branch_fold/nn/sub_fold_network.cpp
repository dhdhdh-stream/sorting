#include "sub_fold_network.h"

using namespace std;

void SubFoldNetwork::construct() {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	int sum_size = 0;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		sum_size += this->scope_sizes[sc_index];
	}
	this->hidden = new Layer(LEAKY_LAYER, 10*sum_size);
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

SubFoldNetwork::SubFoldNetwork(vector<int> scope_sizes,
							   int output_size) {
	this->scope_sizes = scope_sizes;
	this->output_size = output_size;

	this->fold_index = -1;

	construct();
}

SubFoldNetwork::SubFoldNetwork(ifstream& input_file) {
	string num_scope_sizes_line;
	getline(input_file, num_scope_sizes_line);
	int num_scope_sizes = stoi(num_scope_sizes_line);
	this->scope_sizes.reserve(num_scope_sizes);
	for (int sc_index = 0; sc_index < num_scope_sizes; sc_index++) {
		string scope_size_line;
		getline(input_file, scope_size_line);
		this->scope_sizes.push_back(stoi(scope_size_line));
	}
	string output_size_line;
	getline(input_file, output_size_line);
	this->output_size = stoi(output_size_line);

	string fold_index_line;
	getline(input_file, fold_index_line);
	this->fold_index = stoi(fold_index_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

SubFoldNetwork::SubFoldNetwork(SubFoldNetwork* original) {
	this->scope_sizes = original->scope_sizes;
	this->output_size = original->output_size;

	this->fold_index = original->fold_index;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

SubFoldNetwork::~SubFoldNetwork() {
	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}

	delete this->hidden;
	delete this->output;
}

void SubFoldNetwork::add_state(int layer,
							   int num_state) {
	this->hidden->subfold_add_state(layer,
									num_state);
}

void SubFoldNetwork::activate(vector<vector<double>>& state_vals) {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		if (this->fold_index >= sc_index) {
			for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
				this->state_inputs[sc_index]->acti_vals[st_index] = 0.0;
			}
		} else {
			for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
				this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
			}
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void SubFoldNetwork::activate(vector<vector<double>>& state_vals,
							  vector<AbstractNetworkHistory*>& network_historys) {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		if (this->fold_index >= sc_index) {
			for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
				this->state_inputs[sc_index]->acti_vals[st_index] = 0.0;
			}
		} else {
			for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
				this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
			}
		}
	}

	this->hidden->activate();
	this->output->activate();

	SubFoldNetworkHistory* network_history = new SubFoldNetworkHistory(this);
	network_historys.push_back(network_history);
}

void SubFoldNetwork::backprop_weights_with_no_error_signal(
		vector<double>& errors,
		double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

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

void SubFoldNetwork::backprop_new_state(int layer,
										int new_input_size,
										vector<double>& errors,
										double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->subfold_backprop_new_state(layer, new_input_size);

	this->epoch_iter++;
	if (this->epoch_iter == 100) {
		double hidden_max_update = 0.0;
		this->hidden->subfold_get_max_update_new_state(layer,
													   new_input_size,
													   hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
		if (hidden_learning_rate*hidden_max_update > target_max_update) {
			hidden_learning_rate = target_max_update/hidden_max_update;
		}
		this->hidden->subfold_update_weights_new_state(layer,
													   new_input_size,
													   hidden_learning_rate);

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

void SubFoldNetwork::backprop(vector<double>& errors,
							  double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->subfold_backprop(this->fold_index);

	this->epoch_iter++;
	if (this->epoch_iter == 100) {
		double hidden_max_update = 0.0;
		this->hidden->subfold_get_max_update(this->fold_index,
											 hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
		if (hidden_learning_rate*hidden_max_update > target_max_update) {
			hidden_learning_rate = target_max_update/hidden_max_update;
		}
		this->hidden->subfold_update_weights(this->fold_index,
											 hidden_learning_rate);

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

void SubFoldNetwork::save(ofstream& output_file) {
	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

SubFoldNetworkHistory::SubFoldNetworkHistory(SubFoldNetwork* network) {
	this->network = network;

	this->state_inputs_historys.reserve(network->state_inputs.size());
	for (int sc_index = 0; sc_index < (int)network->state_inputs.size(); sc_index++) {
		this->state_inputs_historys.push_back(vector<double>(network->state_inputs[sc_index]->acti_vals.size()));
		for (int st_index = 0; st_index < (int)network->state_inputs[sc_index]->acti_vals.size(); st_index++) {
			this->state_inputs_historys[sc_index][st_index] = network->state_inputs[sc_index]->acti_vals[st_index];
		}
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

void SubFoldNetworkHistory::reset_weights() {
	SubFoldNetwork* network = (SubFoldNetwork*)this->network;

	for (int sc_index = 0; sc_index < (int)network->state_inputs.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)network->state_inputs[sc_index]->acti_vals.size(); st_index++) {
			network->state_inputs[sc_index]->acti_vals[st_index] = this->state_inputs_historys[sc_index][st_index];
		}
	}

	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		network->output->acti_vals[n_index] = this->output_history[n_index];
	}
}
