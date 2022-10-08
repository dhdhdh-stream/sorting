#include "fold_loop_init_network.h"

#include <iostream>

using namespace std;

void FoldLoopInitNetwork::construct() {
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		this->pre_loop_flat_inputs.push_back(new Layer(LINEAR_LAYER, this->pre_loop_flat_sizes[f_index]));
	}

	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	int sum_size = 0;
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		sum_size += this->pre_loop_flat_sizes[f_index];
	}
	sum_size += this->loop_state_size;
	this->hidden = new Layer(LEAKY_LAYER, 4*sum_size*sum_size);
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		this->hidden->input_layers.push_back(this->pre_loop_flat_inputs[f_index]);
	}
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[sc_index]);
	}
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, this->loop_state_size);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

FoldLoopInitNetwork::FoldLoopInitNetwork(vector<int> pre_loop_flat_sizes,
										 int loop_state_size) {
	this->pre_loop_flat_sizes = pre_loop_flat_sizes;
	this->loop_state_size = loop_state_size;

	this->outer_fold_index = -1;

	construct();
}

FoldLoopInitNetwork::FoldLoopInitNetwork(ifstream& input_file) {
	string num_pre_loop_flat_sizes_line;
	getline(input_file, num_pre_loop_flat_sizes_line);
	int num_pre_loop_flat_sizes = stoi(num_pre_loop_flat_sizes_line);
	for (int f_index = 0; f_index < num_pre_loop_flat_sizes; f_index++) {
		string flat_size_line;
		getline(input_file, flat_size_line);
		this->pre_loop_flat_sizes.push_back(stoi(flat_size_line));
	}

	string loop_state_size_line;
	getline(input_file, loop_state_size_line);
	this->loop_state_size = stoi(loop_state_size_line);

	string outer_fold_index_line;
	getline(input_file, outer_fold_index_line);
	this->outer_fold_index = stoi(outer_fold_index_line);

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

FoldLoopInitNetwork::FoldLoopInitNetwork(FoldLoopInitNetwork* original) {
	this->pre_loop_flat_sizes = original->pre_loop_flat_sizes;
	this->loop_state_size = original->loop_state_size;

	this->outer_fold_index = original->outer_fold_index;

	this->scope_sizes = original->scope_sizes;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

FoldLoopInitNetwork::~FoldLoopInitNetwork() {
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_inputs.size(); f_index++) {
		delete this->pre_loop_flat_inputs[f_index];
	}
	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}

	delete this->hidden;
	delete this->output;
}

void FoldLoopInitNetwork::activate(vector<vector<double>>& pre_loop_flat_vals) {
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		for (int s_index = 0; s_index < this->pre_loop_flat_sizes[f_index]; s_index++) {
			this->pre_loop_flat_inputs[f_index]->acti_vals[s_index] = pre_loop_flat_vals[f_index][s_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldLoopInitNetwork::backprop(vector<double>& errors,
								   double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}
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

void FoldLoopInitNetwork::add_scope(int scope_size) {
	this->scope_sizes.push_back(scope_size);
	this->state_inputs.push_back(new Layer(LINEAR_LAYER, scope_size));
	this->hidden->fold_add_scope(this->state_inputs.back());
}

void FoldLoopInitNetwork::pop_scope() {
	this->scope_sizes.pop_back();
	delete this->state_inputs.back();
	this->state_inputs.pop_back();
	this->hidden->fold_pop_scope();
}

void FoldLoopInitNetwork::reset_last() {
	this->hidden->fold_pop_scope();
	this->hidden->fold_add_scope(this->state_inputs.back());
}

void FoldLoopInitNetwork::init_activate(vector<vector<double>>& pre_loop_flat_vals,
										vector<vector<double>>& state_vals) {
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		if (this->outer_fold_index >= f_index) {
			for (int s_index = 0; s_index < this->pre_loop_flat_sizes[f_index]; s_index++) {
				this->pre_loop_flat_inputs[f_index]->acti_vals[s_index] = 0.0;
			}
		} else {
			for (int s_index = 0; s_index < this->pre_loop_flat_sizes[f_index]; s_index++) {
				this->pre_loop_flat_inputs[f_index]->acti_vals[s_index] = pre_loop_flat_vals[f_index][s_index];
			}
		}
	}

	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldLoopInitNetwork::backprop_last_state(vector<double>& errors,
											  double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}
	this->output->backprop();

	this->hidden->fold_backprop_last_state();

	this->epoch_iter++;
	if (this->epoch_iter == 100) {
		double hidden_max_update = 0.0;
		this->hidden->fold_get_max_update_last_state(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
		if (hidden_learning_rate*hidden_max_update > target_max_update) {
			hidden_learning_rate = target_max_update/hidden_max_update;
		}
		this->hidden->fold_update_weights_last_state(hidden_learning_rate);

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

void FoldLoopInitNetwork::backprop_full_state(vector<double>& errors,
											  double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}
	this->output->backprop();

	this->hidden->fold_backprop_full_state((int)this->scope_sizes.size());

	this->epoch_iter++;
	if (this->epoch_iter == 100) {
		double hidden_max_update = 0.0;
		this->hidden->fold_get_max_update_full_state((int)this->scope_sizes.size(),
													 hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
		if (hidden_learning_rate*hidden_max_update > target_max_update) {
			hidden_learning_rate = target_max_update/hidden_max_update;
		}
		this->hidden->fold_update_weights_full_state((int)this->scope_sizes.size(),
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

void FoldLoopInitNetwork::save(ofstream& output_file) {
	output_file << this->pre_loop_flat_sizes.size() << endl;
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		output_file << this->pre_loop_flat_sizes[f_index] << endl;
	}
	output_file << this->loop_state_size << endl;

	output_file << this->outer_fold_index << endl;

	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

FoldLoopInitNetworkHistory::FoldLoopInitNetworkHistory(FoldLoopInitNetwork* network) {
	this->network = network;

	this->pre_loop_flat_inputs_historys.reserve(network->pre_loop_flat_inputs.size());
	for (int f_index = 0; f_index < (int)network->pre_loop_flat_inputs.size(); f_index++) {
		this->pre_loop_flat_inputs_historys.push_back(vector<double>(network->pre_loop_flat_inputs[f_index]->acti_vals.size()));
		for (int n_index = 0; n_index < (int)network->pre_loop_flat_inputs[f_index]->acti_vals.size(); n_index++) {
			this->pre_loop_flat_inputs_historys[f_index][n_index] = network->pre_loop_flat_inputs[f_index]->acti_vals[n_index];
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

void FoldLoopInitNetworkHistory::reset_weights() {
	FoldLoopInitNetwork* network = (FoldLoopInitNetwork*)this->network;

	for (int f_index = 0; f_index < (int)network->pre_loop_flat_inputs.size(); f_index++) {
		for (int n_index = 0; n_index < (int)network->pre_loop_flat_inputs[f_index]->acti_vals.size(); n_index++) {
			network->pre_loop_flat_inputs[f_index]->acti_vals[n_index] = this->pre_loop_flat_inputs_historys[f_index][n_index];
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