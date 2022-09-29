#include "fold_loop_network.h"

#include <iostream>

using namespace std;

void FoldLoopNetwork::construct() {
	this->pre_loop_flat_input = new Layer(LINEAR_LAYER, this->pre_loop_flat_size);
	this->loop_flat_input = new Layer(LINEAR_LAYER, this->loop_flat_size);
	this->post_loop_flat_input = new Layer(LINEAR_LAYER, this->post_loop_flat_size);
	this->obs_input = new Layer(LINEAR_LAYER, this->obs_size);
	this->score_state_input = new Layer(LINEAR_LAYER, 1);
	this->loop_state_input = new Layer(LINEAR_LAYER, this->loop_state_size);

	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	int sum_size = this->pre_loop_flat_size
				   +this->loop_flat_size
				   +this->post_loop_flat_size
				   +this->obs_size
				   +1
				   +this->loop_state_size;
	this->hidden = new Layer(LEAKY_LAYER, 4*sum_size*sum_size);
	this->hidden->input_layers.push_back(this->pre_loop_flat_input);
	this->hidden->input_layers.push_back(this->loop_flat_input);
	this->hidden->input_layers.push_back(this->post_loop_flat_input);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->input_layers.push_back(this->score_state_input);
	this->hidden->input_layers.push_back(this->loop_state_input);
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[sc_index]);
	}
	this->hidden->setup_weights_full();

	this->loop_state_output = new Layer(LINEAR_LAYER, this->loop_state_size);
	this->loop_state_output->input_layers.push_back(this->hidden);
	this->loop_state_output->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->loop_state_output_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

FoldLoopNetwork::FoldLoopNetwork(int pre_loop_flat_size,
								 int loop_flat_size,
								 int post_loop_flat_size,
								 int obs_size,
								 int loop_state_size) {
	this->pre_loop_flat_size = pre_loop_flat_size;
	this->loop_flat_size = loop_flat_size;
	this->post_loop_flat_size = post_loop_flat_size;
	this->obs_size = obs_size;
	this->loop_state_size = loop_state_size;

	this->fold_index = -1;
	this->average_error = -1.0;

	construct();
}

FoldLoopNetwork::FoldLoopNetwork(ifstream& input_file) {
	string pre_loop_flat_size_line;
	getline(input_file, pre_loop_flat_size_line);
	this->pre_loop_flat_size = stoi(pre_loop_flat_size_line);

	string loop_flat_size_line;
	getline(input_file, loop_flat_size_line);
	this->loop_flat_size = stoi(loop_flat_size_line);

	string post_loop_flat_size_line;
	getline(input_file, post_loop_flat_size_line);
	this->post_loop_flat_size = stoi(post_loop_flat_size_line);

	string obs_size_line;
	getline(input_file, obs_size_line);
	this->obs_size = stoi(obs_size_line);

	string loop_state_size_line;
	getline(input_file, loop_state_size_line);
	this->loop_state_size = stoi(loop_state_size_line);

	string fold_index_line;
	getline(input_file, fold_index_line);
	this->fold_index = stoi(fold_index_line);

	string average_error_line;
	getline(input_file, average_error_line);
	this->average_error = stof(average_error_line);

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
	this->loop_state_output->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

FoldLoopNetwork::FoldLoopNetwork(FoldLoopNetwork* original) {
	this->pre_loop_flat_size = original->pre_loop_flat_size;
	this->loop_flat_size = original->loop_flat_size;
	this->post_loop_flat_size = original->post_loop_flat_size;
	this->obs_size = original->obs_size;
	this->loop_state_size = original->loop_state_size;

	this->fold_index = original->fold_index;
	this->average_error = original->average_error;

	this->scope_sizes = original->scope_sizes;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->loop_state_output->copy_weights_from(original->loop_state_output);
	this->output->copy_weights_from(original->output);
}

FoldLoopNetwork::~FoldLoopNetwork() {
	delete this->pre_loop_flat_input;
	delete this->loop_flat_input;
	delete this->post_loop_flat_input;
	delete this->obs_input;
	delete this->score_state_input;
	delete this->loop_state_input;
	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}

	delete this->hidden;
	delete this->loop_state_output;
	delete this->output;
}

void FoldLoopNetwork::activate(double* pre_loop_flat_inputs,
							   double* loop_flat_inputs,
							   double* post_loop_flat_inputs,
							   vector<double>& obs,
							   double& score_state,
							   vector<double>& loop_state) {
	for (int f_index = 0; f_index < this->pre_loop_flat_size; f_index++) {
		this->pre_loop_flat_input->acti_vals[f_index] = pre_loop_flat_inputs[f_index];
	}
	for (int f_index = 0; f_index < this->loop_flat_size; f_index++) {
		this->loop_flat_input->acti_vals[f_index] = loop_flat_inputs[f_index];
	}
	for (int f_index = 0; f_index < this->post_loop_flat_size; f_index++) {
		this->post_loop_flat_input->acti_vals[f_index] = post_loop_flat_inputs[f_index];
	}

	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = obs[o_index];
	}

	this->score_state_input->acti_vals[0] = score_state;
	for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
		this->loop_state_input->acti_vals[s_index] = loop_state[s_index];
	}

	this->hidden->activate();
	this->loop_state_output->activate();
	this->output->activate();
}

void FoldLoopNetwork::activate(double* pre_loop_flat_inputs,
							   double* loop_flat_inputs,
							   double* post_loop_flat_inputs,
							   vector<double>& obs,
							   double& score_state,
							   vector<double>& loop_state,
							   vector<AbstractNetworkHistory*>& network_historys) {
	for (int f_index = 0; f_index < this->pre_loop_flat_size; f_index++) {
		this->pre_loop_flat_input->acti_vals[f_index] = pre_loop_flat_inputs[f_index];
	}
	for (int f_index = 0; f_index < this->loop_flat_size; f_index++) {
		this->loop_flat_input->acti_vals[f_index] = loop_flat_inputs[f_index];
	}
	for (int f_index = 0; f_index < this->post_loop_flat_size; f_index++) {
		this->post_loop_flat_input->acti_vals[f_index] = post_loop_flat_inputs[f_index];
	}

	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = obs[o_index];
	}

	this->score_state_input->acti_vals[0] = score_state;
	for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
		this->loop_state_input->acti_vals[s_index] = loop_state[s_index];
	}

	this->hidden->activate();
	this->loop_state_output->activate();
	this->output->activate();

	FoldLoopNetworkHistory* network_history = new FoldLoopNetworkHistory(this);
	network_historys.push_back(network_history);
}

void FoldLoopNetwork::backprop(vector<double>& errors,
							   vector<double>& loop_state_errors,
							   double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}
	this->output->backprop();

	for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
		this->loop_state_output->errors[s_index] = loop_state_errors[s_index];
	}
	this->loop_state_output->backprop();

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

		double loop_state_output_max_update = 0.0;
		this->loop_state_output->get_max_update(loop_state_output_max_update);
		this->loop_state_output_average_max_update = 0.999*this->loop_state_output_average_max_update+0.001*loop_state_output_max_update;
		double loop_state_output_learning_rate = (0.3*target_max_update)/this->loop_state_output_average_max_update;
		if (loop_state_output_learning_rate*loop_state_output_max_update > target_max_update) {
			loop_state_output_learning_rate = target_max_update/loop_state_output_max_update;
		}
		this->loop_state_output->update_weights(loop_state_output_learning_rate);

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

void FoldLoopNetwork::add_scope(int scope_size) {
	this->scope_sizes.push_back(scope_size);
	this->state_inputs.push_back(new Layer(LINEAR_LAYER, scope_size));
	this->hidden->fold_add_scope(this->state_inputs.back());
}

void FoldLoopNetwork::pop_scope() {
	this->scope_sizes.pop_back();
	delete this->state_inputs.back();
	this->state_inputs.pop_back();
	this->hidden->fold_pop_scope();
}

void FoldLoopNetwork::reset_last() {
	this->hidden->fold_pop_scope();
	this->hidden->fold_add_scope(this->state_inputs.back());
}

void FoldLoopNetwork::set_just_score() {
	while (this->state_inputs.size() > 1) {
		this->scope_sizes.pop_back();
		delete this->state_inputs.back();
		this->state_inputs.pop_back();
		this->hidden->fold_pop_scope();
	}

	reset_last();
}

void FoldLoopNetwork::set_can_compress() {
	int sum_scope_sizes = 0;
	while (this->state_inputs.size() > 1) {
		sum_scope_sizes += this->scope_sizes.back();
		this->scope_sizes.pop_back();
		delete this->state_inputs.back();
		this->state_inputs.pop_back();
		this->hidden->fold_pop_scope();
	}

	this->scope_sizes.push_back(sum_scope_sizes-1);
	this->state_inputs.push_back(new Layer(LINEAR_LAYER, sum_scope_sizes-1));
	this->hidden->fold_add_scope(this->state_inputs.back());
}

void FoldLoopNetwork::activate(double* pre_loop_flat_inputs,
							   double* loop_flat_inputs,
							   double* post_loop_flat_inputs,
							   vector<double>& obs,
							   double& score_state,
							   vector<double>& loop_state,
							   vector<vector<double>>& state_vals) {
	for (int f_index = 0; f_index < this->pre_loop_flat_size; f_index++) {
		if (this->fold_index >= f_index) {
			this->pre_loop_flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->pre_loop_flat_input->acti_vals[f_index] = pre_loop_flat_inputs[f_index];
		}
	}
	for (int f_index = 0; f_index < this->loop_flat_size; f_index++) {
		if (this->fold_index >= this->pre_loop_flat_size + f_index) {
			this->loop_flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->loop_flat_input->acti_vals[f_index] = loop_flat_inputs[f_index];
		}
	}
	for (int f_index = 0; f_index < this->post_loop_flat_size; f_index++) {
		if (this->fold_index >= this->pre_loop_flat_size + this->loop_flat_size + f_index) {
			this->post_loop_flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->post_loop_flat_input->acti_vals[f_index] = post_loop_flat_inputs[f_index];
		}
	}

	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = obs[o_index];
	}

	this->score_state_input->acti_vals[0] = score_state;
	for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
		this->loop_state_input->acti_vals[s_index] = loop_state[s_index];
	}

	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}

	this->hidden->activate();
	this->loop_state_output->activate();
	this->output->activate();
}

void FoldLoopNetwork::activate(double* pre_loop_flat_inputs,
							   double* loop_flat_inputs,
							   double* post_loop_flat_inputs,
							   vector<double>& obs,
							   double& score_state,
							   vector<double>& loop_state,
							   vector<vector<double>>& state_vals,
							   vector<AbstractNetworkHistory*>& network_historys) {
	for (int f_index = 0; f_index < this->pre_loop_flat_size; f_index++) {
		if (this->fold_index >= f_index) {
			this->pre_loop_flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->pre_loop_flat_input->acti_vals[f_index] = pre_loop_flat_inputs[f_index];
		}
	}
	for (int f_index = 0; f_index < this->loop_flat_size; f_index++) {
		if (this->fold_index >= this->pre_loop_flat_size + f_index) {
			this->loop_flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->loop_flat_input->acti_vals[f_index] = loop_flat_inputs[f_index];
		}
	}
	for (int f_index = 0; f_index < this->post_loop_flat_size; f_index++) {
		if (this->fold_index >= this->pre_loop_flat_size + this->loop_flat_size + f_index) {
			this->post_loop_flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->post_loop_flat_input->acti_vals[f_index] = post_loop_flat_inputs[f_index];
		}
	}

	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = obs[o_index];
	}

	this->score_state_input->acti_vals[0] = score_state;
	for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
		this->loop_state_input->acti_vals[s_index] = loop_state[s_index];
	}

	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}

	this->hidden->activate();
	this->loop_state_output->activate();
	this->output->activate();

	FoldLoopNetworkHistory* network_history = new FoldLoopNetworkHistory(this);
	network_historys.push_back(network_history);
}

void FoldLoopNetwork::backprop_last_state(vector<double>& errors,
										  vector<double>& loop_state_errors,
										  double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}
	this->output->backprop();

	for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
		this->loop_state_output->errors[s_index] = loop_state_errors[s_index];
	}
	this->loop_state_output->backprop();

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

		double loop_state_output_max_update = 0.0;
		this->loop_state_output->get_max_update(loop_state_output_max_update);
		this->loop_state_output_average_max_update = 0.999*this->loop_state_output_average_max_update+0.001*loop_state_output_max_update;
		double loop_state_output_learning_rate = (0.3*target_max_update)/this->loop_state_output_average_max_update;
		if (loop_state_output_learning_rate*loop_state_output_max_update > target_max_update) {
			loop_state_output_learning_rate = target_max_update/loop_state_output_max_update;
		}
		this->loop_state_output->update_weights(loop_state_output_learning_rate);

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

void FoldLoopNetwork::backprop_full_state(vector<double>& errors,
										  vector<double>& loop_state_errors,
										  double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}
	this->output->backprop();

	for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
		this->loop_state_output->errors[s_index] = loop_state_errors[s_index];
	}
	this->loop_state_output->backprop();

	this->hidden->fold_loop_backprop_full_state();

	this->epoch_iter++;
	if (this->epoch_iter == 100) {
		double hidden_max_update = 0.0;
		this->hidden->fold_loop_get_max_update_full_state(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
		if (hidden_learning_rate*hidden_max_update > target_max_update) {
			hidden_learning_rate = target_max_update/hidden_max_update;
		}
		this->hidden->fold_loop_update_weights_full_state(hidden_learning_rate);

		double loop_state_output_max_update = 0.0;
		this->loop_state_output->get_max_update(loop_state_output_max_update);
		this->loop_state_output_average_max_update = 0.999*this->loop_state_output_average_max_update+0.001*loop_state_output_max_update;
		double loop_state_output_learning_rate = (0.3*target_max_update)/this->loop_state_output_average_max_update;
		if (loop_state_output_learning_rate*loop_state_output_max_update > target_max_update) {
			loop_state_output_learning_rate = target_max_update/loop_state_output_max_update;
		}
		this->loop_state_output->update_weights(loop_state_output_learning_rate);

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

void FoldLoopNetwork::save(ofstream& output_file) {
	output_file << this->pre_loop_flat_size << endl;
	output_file << this->loop_flat_size << endl;
	output_file << this->post_loop_flat_size << endl;
	output_file << this->obs_size << endl;
	output_file << this->loop_state_size << endl;

	output_file << this->fold_index << endl;
	output_file << this->average_error << endl;

	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->loop_state_output->save_weights(output_file);
	this->output->save_weights(output_file);
}

FoldLoopNetworkHistory::FoldLoopNetworkHistory(FoldLoopNetwork* network) {
	this->network = network;

	this->pre_loop_flat_input_history.reserve(network->pre_loop_flat_input->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->pre_loop_flat_input->acti_vals.size(); n_index++) {
		this->pre_loop_flat_input_history.push_back(network->pre_loop_flat_input->acti_vals[n_index]);
	}
	this->loop_flat_input_history.reserve(network->loop_flat_input->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->loop_flat_input->acti_vals.size(); n_index++) {
		this->loop_flat_input_history.push_back(network->loop_flat_input->acti_vals[n_index]);
	}
	this->post_loop_flat_input_history.reserve(network->post_loop_flat_input->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->post_loop_flat_input->acti_vals.size(); n_index++) {
		this->post_loop_flat_input_history.push_back(network->post_loop_flat_input->acti_vals[n_index]);
	}
	this->obs_input_history.reserve(network->obs_input->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->obs_input->acti_vals.size(); n_index++) {
		this->obs_input_history.push_back(network->obs_input->acti_vals[n_index]);
	}
	this->score_state_input_history.push_back(network->score_state_input->acti_vals[0]);
	this->loop_state_input_history.reserve(network->loop_state_input->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->loop_state_input->acti_vals.size(); n_index++) {
		this->loop_state_input_history.push_back(network->loop_state_input->acti_vals[n_index]);
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
	this->loop_state_output_history.reserve(network->loop_state_output->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->loop_state_output->acti_vals.size(); n_index++) {
		this->loop_state_output_history.push_back(network->loop_state_output->acti_vals[n_index]);
	}
	this->output_history.reserve(network->output->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		this->output_history.push_back(network->output->acti_vals[n_index]);
	}
}

void FoldLoopNetworkHistory::reset_weights() {
	FoldLoopNetwork* network = (FoldLoopNetwork*)this->network;

	for (int n_index = 0; n_index < (int)network->pre_loop_flat_input->acti_vals.size(); n_index++) {
		network->pre_loop_flat_input->acti_vals[n_index] = this->pre_loop_flat_input_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->loop_flat_input->acti_vals.size(); n_index++) {
		network->loop_flat_input->acti_vals[n_index] = this->loop_flat_input_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->post_loop_flat_input->acti_vals.size(); n_index++) {
		network->post_loop_flat_input->acti_vals[n_index] = this->post_loop_flat_input_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->obs_input->acti_vals.size(); n_index++) {
		network->obs_input->acti_vals[n_index] = this->obs_input_history[n_index];
	}
	network->score_state_input->acti_vals[0] = this->score_state_input_history[0];
	for (int n_index = 0; n_index < (int)network->loop_state_input->acti_vals.size(); n_index++) {
		network->loop_state_input->acti_vals[n_index] = this->loop_state_input_history[n_index];
	}

	for (int sc_index = 0; sc_index < (int)network->state_inputs.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)network->state_inputs[sc_index]->acti_vals.size(); st_index++) {
			network->state_inputs[sc_index]->acti_vals[st_index] = this->state_inputs_historys[sc_index][st_index];
		}
	}

	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->loop_state_output->acti_vals.size(); n_index++) {
		network->loop_state_output->acti_vals[n_index] = this->loop_state_output_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		network->output->acti_vals[n_index] = this->output_history[n_index];
	}
}
