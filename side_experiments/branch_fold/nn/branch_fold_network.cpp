#include "branch_fold_network.h"

#include <iostream>

using namespace std;

void BranchFoldNetwork::construct() {
	for (int f_index = 0; f_index < (int)this->pre_branch_flat_sizes.size(); f_index++) {
		this->pre_branch_flat_inputs.push_back(new Layer(LINEAR_LAYER, this->pre_branch_flat_sizes[f_index]));
	}
	for (int f_index = 0; f_index < (int)this->new_branch_flat_sizes.size(); f_index++) {
		this->new_branch_flat_inputs.push_back(new Layer(LINEAR_LAYER, this->new_branch_flat_sizes[f_index]));
	}
	for (int f_index = 0; f_index < (int)this->post_branch_flat_sizes.size(); f_index++) {
		this->post_branch_flat_inputs.push_back(new Layer(LINEAR_LAYER, this->post_branch_flat_sizes[f_index]));
	}

	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}

	this->hidden = new Layer(LEAKY_LAYER, 500);
	for (int f_index = 0; f_index < (int)this->pre_branch_flat_sizes.size(); f_index++) {
		this->hidden->input_layers.push_back(this->pre_branch_flat_inputs[f_index]);
	}
	for (int f_index = 0; f_index < (int)this->new_branch_flat_sizes.size(); f_index++) {
		this->hidden->input_layers.push_back(this->new_branch_flat_inputs[f_index]);
	}
	for (int f_index = 0; f_index < (int)this->post_branch_flat_sizes.size(); f_index++) {
		this->hidden->input_layers.push_back(this->post_branch_flat_inputs[f_index]);
	}
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[sc_index]);
	}
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

BranchFoldNetwork::BranchFoldNetwork(vector<int> pre_branch_flat_sizes,
									 vector<int> new_branch_flat_sizes,
									 vector<int> post_branch_flat_sizes) {
	this->pre_branch_flat_sizes = pre_branch_flat_sizes;
	this->new_branch_flat_sizes = new_branch_flat_sizes;
	this->post_branch_flat_sizes = post_branch_flat_sizes;

	this->fold_index = -1;

	construct();
}

BranchFoldNetwork::BranchFoldNetwork(ifstream& input_file) {
	string num_pre_branch_flat_sizes_line;
	getline(input_file, num_pre_branch_flat_sizes_line);
	int num_pre_branch_flat_sizes = stoi(num_pre_branch_flat_sizes_line);
	for (int f_index = 0; f_index < num_pre_branch_flat_sizes; f_index++) {
		string flat_size_line;
		getline(input_file, flat_size_line);
		this->pre_branch_flat_sizes.push_back(stoi(flat_size_line));
	}

	string num_new_branch_flat_sizes_line;
	getline(input_file, num_new_branch_flat_sizes_line);
	int num_new_branch_flat_sizes = stoi(num_new_branch_flat_sizes_line);
	for (int f_index = 0; f_index < num_new_branch_flat_sizes; f_index++) {
		string flat_size_line;
		getline(input_file, flat_size_line);
		this->new_branch_flat_sizes.push_back(stoi(flat_size_line));
	}

	string num_post_branch_flat_sizes_line;
	getline(input_file, num_post_branch_flat_sizes_line);
	int num_post_branch_flat_sizes = stoi(num_post_branch_flat_sizes_line);
	for (int f_index = 0; f_index < num_post_branch_flat_sizes; f_index++) {
		string flat_size_line;
		getline(input_file, flat_size_line);
		this->post_branch_flat_sizes.push_back(stoi(flat_size_line));
	}

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

BranchFoldNetwork::BranchFoldNetwork(BranchFoldNetwork* original) {
	this->pre_branch_flat_sizes = original->pre_branch_flat_sizes;
	this->new_branch_flat_sizes = original->new_branch_flat_sizes;
	this->post_branch_flat_sizes = original->post_branch_flat_sizes;

	this->fold_index = original->fold_index;

	this->scope_sizes = original->scope_sizes;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

BranchFoldNetwork::~BranchFoldNetwork() {
	for (int f_index = 0; f_index < (int)this->pre_branch_flat_sizes.size(); f_index++) {
		delete this->pre_branch_flat_inputs[f_index];
	}
	for (int f_index = 0; f_index < (int)this->new_branch_flat_sizes.size(); f_index++) {
		delete this->new_branch_flat_inputs[f_index];
	}
	for (int f_index = 0; f_index < (int)this->post_branch_flat_sizes.size(); f_index++) {
		delete this->post_branch_flat_inputs[f_index];
	}

	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}

	delete this->hidden;
	delete this->output;
}

void BranchFoldNetwork::activate(vector<vector<double>>& pre_branch_flat_vals,
								 vector<vector<double>>& new_branch_flat_vals,
								 vector<vector<double>>& post_branch_flat_vals) {
	for (int f_index = 0; f_index < (int)this->pre_branch_flat_sizes.size(); f_index++) {
		for (int s_index = 0; s_index < this->pre_branch_flat_sizes[f_index]; s_index++) {
			this->pre_branch_flat_inputs[f_index]->acti_vals[s_index] = pre_branch_flat_vals[f_index][s_index];
		}
	}
	for (int f_index = 0; f_index < (int)this->new_branch_flat_sizes.size(); f_index++) {
		for (int s_index = 0; s_index < this->new_branch_flat_sizes[f_index]; s_index++) {
			this->new_branch_flat_inputs[f_index]->acti_vals[s_index] = new_branch_flat_vals[f_index][s_index];
		}
	}
	for (int f_index = 0; f_index < (int)this->post_branch_flat_sizes.size(); f_index++) {
		for (int s_index = 0; s_index < this->post_branch_flat_sizes[f_index]; s_index++) {
			this->post_branch_flat_inputs[f_index]->acti_vals[s_index] = post_branch_flat_vals[f_index][s_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void BranchFoldNetwork::backprop(vector<double>& errors,
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

void BranchFoldNetwork::save(ofstream& output_file) {
	output_file << this->pre_branch_flat_sizes.size() << endl;
	for (int f_index = 0; f_index < (int)this->pre_branch_flat_sizes.size(); f_index++) {
		output_file << this->pre_branch_flat_sizes[f_index] << endl;
	}
	output_file << this->new_branch_flat_sizes.size() << endl;
	for (int f_index = 0; f_index < (int)this->new_branch_flat_sizes.size(); f_index++) {
		output_file << this->new_branch_flat_sizes[f_index] << endl;
	}
	output_file << this->post_branch_flat_sizes.size() << endl;
	for (int f_index = 0; f_index < (int)this->post_branch_flat_sizes.size(); f_index++) {
		output_file << this->post_branch_flat_sizes[f_index] << endl;
	}

	output_file << this->fold_index << endl;

	output_file << this->scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		output_file << this->scope_sizes[sc_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}
