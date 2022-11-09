#include "fold_network.h"

#include <iostream>

using namespace std;

void FoldNetwork::construct() {
	for (int f_index = 0; f_index < (int)this->flat_sizes.size(); f_index++) {
		this->flat_inputs.push_back(new Layer(LINEAR_LAYER, this->flat_sizes[f_index]));
	}

	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	this->hidden = new Layer(LEAKY_LAYER, 100);
	for (int f_index = 0; f_index < (int)this->flat_sizes.size(); f_index++) {
		this->hidden->input_layers.push_back(this->flat_inputs[f_index]);
	}
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
						 int output_size) {
	this->flat_sizes = flat_sizes;
	this->output_size = output_size;

	this->fold_index = -1;

	construct();
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
	this->flat_sizes = original->flat_sizes;
	this->output_size = original->output_size;

	this->fold_index = original->fold_index;

	this->scope_sizes = original->scope_sizes;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

FoldNetwork::~FoldNetwork() {
	for (int f_index = 0; f_index < (int)this->flat_sizes.size(); f_index++) {
		delete this->flat_inputs[f_index];
	}
	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}

	delete this->hidden;
	delete this->output;
}

void FoldNetwork::activate(vector<vector<double>>& flat_vals) {
	for (int f_index = 0; f_index < (int)this->flat_sizes.size(); f_index++) {
		for (int s_index = 0; s_index < this->flat_sizes[f_index]; s_index++) {
			this->flat_inputs[f_index]->acti_vals[s_index] = flat_vals[f_index][s_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldNetwork::activate(vector<vector<double>>& flat_vals,
						   vector<AbstractNetworkHistory*>& network_historys) {
	activate(flat_vals);

	FoldNetworkHistory* network_history = new FoldNetworkHistory(this);
	network_historys.push_back(network_history);
}

void FoldNetwork::backprop(vector<double>& errors,
						   double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
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

void FoldNetwork::backprop_weights_with_no_error_signal(vector<double>& errors,
														double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
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

void FoldNetwork::backprop_errors_with_no_weight_change(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();
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

void FoldNetwork::activate(vector<vector<double>>& flat_vals,
						   vector<vector<double>>& state_vals) {
	for (int f_index = this->fold_index+1; f_index < (int)this->flat_sizes.size(); f_index++) {
		for (int s_index = 0; s_index < this->flat_sizes[f_index]; s_index++) {
			this->flat_inputs[f_index]->acti_vals[s_index] = flat_vals[f_index][s_index];
		}
	}

	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}

	this->hidden->fold_activate(this->fold_index);
	this->output->activate();
}

void FoldNetwork::activate(vector<vector<double>>& flat_vals,
						   vector<vector<double>>& state_vals,
						   vector<AbstractNetworkHistory*>& network_historys) {
	activate(flat_vals,
			 state_vals);

	FoldNetworkHistory* network_history = new FoldNetworkHistory(this);
	network_historys.push_back(network_history);
}

void FoldNetwork::backprop_no_state(vector<double>& errors,
									double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->fold_backprop_no_state(this->fold_index,
										 (int)this->scope_sizes.size());

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->fold_get_max_update(this->fold_index,
										  hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->fold_update_weights(this->fold_index,
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
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void FoldNetwork::backprop_last_state(vector<double>& errors,
									  double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->fold_backprop_last_state(this->fold_index,
										   (int)this->scope_sizes.size());

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->fold_get_max_update(this->fold_index,
										  hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->fold_update_weights(this->fold_index,
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
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void FoldNetwork::backprop_last_state_with_no_weight_change(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->fold_backprop_last_state_with_no_weight_change(
		this->fold_index,
		(int)this->scope_sizes.size());
}

// void FoldNetwork::backprop_state(vector<double>& errors,
// 								 double target_max_update) {
// 	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
// 		this->output->errors[e_index] = errors[e_index];
// 	}

// 	this->output->backprop();
// 	this->hidden->fold_backprop_state(this->fold_index,
// 									  (int)this->scope_sizes.size());

// 	this->epoch_iter++;
// 	if (this->epoch_iter == 20) {
// 		double hidden_max_update = 0.0;
// 		this->hidden->fold_get_max_update(this->fold_index,
// 										  hidden_max_update);
// 		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
// 		if (hidden_max_update > 0.0) {
// 			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
// 			if (hidden_learning_rate*hidden_max_update > target_max_update) {
// 				hidden_learning_rate = target_max_update/hidden_max_update;
// 			}
// 			this->hidden->fold_update_weights(this->fold_index,
// 											  hidden_learning_rate);
// 		}

// 		double output_max_update = 0.0;
// 		this->output->get_max_update(output_max_update);
// 		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
// 		if (output_max_update > 0.0) {
// 			double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
// 			if (output_learning_rate*output_max_update > target_max_update) {
// 				output_learning_rate = target_max_update/output_max_update;
// 			}
// 			this->output->update_weights(output_learning_rate);
// 		}

// 		this->epoch_iter = 0;
// 	}
// }

void FoldNetwork::backprop_fold(vector<double>& errors,
								double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->fold_backprop(this->fold_index,
								(int)this->scope_sizes.size());

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->fold_get_max_update(this->fold_index,
										  hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->fold_update_weights(this->fold_index,
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
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void FoldNetwork::save(ofstream& output_file) {
	output_file << this->flat_sizes.size() << endl;
	for (int f_index = 0; f_index < (int)this->flat_sizes.size(); f_index++) {
		output_file << this->flat_sizes[f_index] << endl;
	}
	output_file << this->output_size << endl;

	output_file << this->fold_index << endl;

	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

FoldNetworkHistory::FoldNetworkHistory(FoldNetwork* network) {
	this->network = network;

	this->flat_inputs_historys.reserve(network->flat_inputs.size());
	for (int f_index = 0; f_index < (int)network->flat_inputs.size(); f_index++) {
		this->flat_inputs_historys.push_back(vector<double>(network->flat_inputs[f_index]->acti_vals.size()));
		for (int n_index = 0; n_index < (int)network->flat_inputs[f_index]->acti_vals.size(); n_index++) {
			this->flat_inputs_historys[f_index][n_index] = network->flat_inputs[f_index]->acti_vals[n_index];
		}
	}

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

void FoldNetworkHistory::reset_weights() {
	FoldNetwork* network = (FoldNetwork*)this->network;

	for (int f_index = 0; f_index < (int)network->flat_inputs.size(); f_index++) {
		for (int n_index = 0; n_index < (int)network->flat_inputs[f_index]->acti_vals.size(); n_index++) {
			network->flat_inputs[f_index]->acti_vals[n_index] = this->flat_inputs_historys[f_index][n_index];
		}
	}

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
