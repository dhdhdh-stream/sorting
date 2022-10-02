#include "fold_combine_network.h"

#include <iostream>

using namespace std;

void FoldCombineNetwork::construct() {
	this->loop_state_input = new Layer(LINEAR_LAYER, this->loop_state_size);
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		this->pre_loop_flat_inputs.push_back(new Layer(LINEAR_LAYER, this->pre_loop_flat_sizes[f_index]));
	}
	for (int f_index = 0; f_index < (int)this->post_loop_flat_sizes.size(); f_index++) {
		this->post_loop_flat_inputs.push_back(new Layer(LINEAR_LAYER, this->post_loop_flat_sizes[f_index]));
	}

	for (int sc_index = 0; sc_index < (int)this->outer_scope_sizes.size(); sc_index++) {
		this->outer_state_inputs.push_back(new Layer(LINEAR_LAYER, this->outer_scope_sizes[sc_index]));
	}

	int sum_size = 0;
	sum_size += this->loop_state_size;
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		sum_size += this->pre_loop_flat_sizes[f_index];
	}
	for (int f_index = 0; f_index < (int)this->post_loop_flat_sizes.size(); f_index++) {
		sum_size += this->post_loop_flat_sizes[f_index];
	}
	this->hidden = new Layer(LEAKY_LAYER, 4*sum_size*sum_size);
	// set inputs_layers[0] to be loop_state_input for state backprop
	this->hidden->input_layers.push_back(this->loop_state_input);
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		this->hidden->input_layers.push_back(this->pre_loop_flat_inputs[f_index]);
	}
	for (int f_index = 0; f_index < (int)this->post_loop_flat_sizes.size(); f_index++) {
		this->hidden->input_layers.push_back(this->post_loop_flat_inputs[f_index]);
	}
	for (int sc_index = 0; sc_index < (int)this->outer_scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->outer_state_inputs[sc_index]);
	}
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

FoldCombineNetwork::FoldCombineNetwork(int loop_state_size,
									   vector<int> pre_loop_flat_sizes,
									   vector<int> post_loop_flat_sizes) {
	this->loop_state_size = loop_state_size;
	this->pre_loop_flat_sizes = pre_loop_flat_sizes;
	this->post_loop_flat_sizes = post_loop_flat_sizes;

	this->outer_fold_index = -1;
	this->average_error = -1.0;

	construct();
}

FoldCombineNetwork::FoldCombineNetwork(ifstream& input_file) {
	string loop_state_size_line;
	getline(input_file, loop_state_size_line);
	this->loop_state_size = stoi(loop_state_size_line);

	string num_pre_loop_flat_sizes_line;
	getline(input_file, num_pre_loop_flat_sizes_line);
	int num_pre_loop_flat_sizes = stoi(num_pre_loop_flat_sizes_line);
	for (int f_index = 0; f_index < num_pre_loop_flat_sizes; f_index++) {
		string flat_size_line;
		getline(input_file, flat_size_line);
		this->pre_loop_flat_sizes.push_back(stoi(flat_size_line));
	}

	string num_post_loop_flat_sizes_line;
	getline(input_file, num_post_loop_flat_sizes_line);
	int num_post_loop_flat_sizes = stoi(num_post_loop_flat_sizes_line);
	for (int f_index = 0; f_index < num_post_loop_flat_sizes; f_index++) {
		string flat_size_line;
		getline(input_file, flat_size_line);
		this->post_loop_flat_sizes.push_back(stoi(flat_size_line));
	}

	string outer_fold_index_line;
	getline(input_file, outer_fold_index_line);
	this->outer_fold_index = stoi(outer_fold_index_line);

	string average_error_line;
	getline(input_file, average_error_line);
	this->average_error = stof(average_error_line);

	string num_outer_scope_sizes_line;
	getline(input_file, num_outer_scope_sizes_line);
	int num_outer_scope_sizes = stoi(num_outer_scope_sizes_line);
	for (int sc_index = 0; sc_index < num_outer_scope_sizes; sc_index++) {
		string scope_size_line;
		getline(input_file, scope_size_line);
		this->outer_scope_sizes.push_back(stoi(scope_size_line));
	}

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

FoldCombineNetwork::FoldCombineNetwork(FoldCombineNetwork* original) {
	this->loop_state_size = original->loop_state_size;
	this->pre_loop_flat_sizes = original->pre_loop_flat_sizes;
	this->post_loop_flat_sizes = original->post_loop_flat_sizes;

	this->outer_fold_index = original->outer_fold_index;
	this->average_error = original->average_error;

	this->outer_scope_sizes = original->outer_scope_sizes;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

FoldCombineNetwork::~FoldCombineNetwork() {
	delete this->loop_state_input;
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_inputs.size(); f_index++) {
		delete this->pre_loop_flat_inputs[f_index];
	}
	for (int f_index = 0; f_index < (int)this->post_loop_flat_inputs.size(); f_index++) {
		delete this->post_loop_flat_inputs[f_index];
	}

	for (int sc_index = 0; sc_index < (int)this->outer_state_inputs.size(); sc_index++) {
		delete this->outer_state_inputs[sc_index];
	}

	delete this->hidden;
	delete this->output;
}

void FoldCombineNetwork::activate(vector<double>& loop_state,
								  vector<vector<double>>& pre_loop_flat_vals,
								  vector<vector<double>>& post_loop_flat_vals) {
	for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
		this->loop_state_input->acti_vals[s_index] = loop_state[s_index];
	}
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		for (int s_index = 0; s_index < this->pre_loop_flat_sizes[f_index]; s_index++) {
			this->pre_loop_flat_inputs[f_index]->acti_vals[s_index] = pre_loop_flat_vals[f_index][s_index];
		}
	}
	for (int f_index = 0; f_index < (int)this->post_loop_flat_sizes.size(); f_index++) {
		for (int s_index = 0; s_index < this->post_loop_flat_sizes[f_index]; s_index++) {
			this->post_loop_flat_inputs[f_index]->acti_vals[s_index] = post_loop_flat_vals[f_index][s_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldCombineNetwork::backprop(vector<double>& errors,
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

void FoldCombineNetwork::outer_add_scope(int scope_size) {
	this->outer_scope_sizes.push_back(scope_size);
	this->outer_state_inputs.push_back(new Layer(LINEAR_LAYER, scope_size));
	this->hidden->fold_add_scope(this->outer_state_inputs.back());
}

void FoldCombineNetwork::outer_pop_scope() {
	this->outer_scope_sizes.pop_back();
	delete this->outer_state_inputs.back();
	this->outer_state_inputs.pop_back();
	this->hidden->fold_pop_scope();
}

void FoldCombineNetwork::outer_reset_last() {
	this->hidden->fold_pop_scope();
	this->hidden->fold_add_scope(this->outer_state_inputs.back());
}

void FoldCombineNetwork::outer_set_just_score() {
	while (this->outer_state_inputs.size() > 1) {
		this->outer_scope_sizes.pop_back();
		delete this->outer_state_inputs.back();
		this->outer_state_inputs.pop_back();
		this->hidden->fold_pop_scope();
	}

	outer_reset_last();
}

void FoldCombineNetwork::outer_set_can_compress() {
	int sum_scope_sizes = 0;
	while (this->outer_state_inputs.size() > 1) {
		sum_scope_sizes += this->outer_scope_sizes.back();
		this->outer_scope_sizes.pop_back();
		delete this->outer_state_inputs.back();
		this->outer_state_inputs.pop_back();
		this->hidden->fold_pop_scope();
	}

	this->outer_scope_sizes.push_back(sum_scope_sizes-1);
	this->outer_state_inputs.push_back(new Layer(LINEAR_LAYER, sum_scope_sizes-1));
	this->hidden->fold_add_scope(this->outer_state_inputs.back());
}

void FoldCombineNetwork::outer_activate(vector<double>& loop_state,
										vector<vector<double>>& pre_loop_flat_vals,
										vector<vector<double>>& post_loop_flat_vals,
										vector<vector<double>>& outer_state_vals) {
	if (this->outer_fold_index >= (int)this->pre_loop_flat_sizes.size()) {
		for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
			this->loop_state_input->acti_vals[s_index] = 0.0;
		}
	} else {
		for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
			this->loop_state_input->acti_vals[s_index] = loop_state[s_index];
		}
	}

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

	for (int f_index = 0; f_index < (int)this->post_loop_flat_sizes.size(); f_index++) {
		if (this->outer_fold_index >= (int)this->pre_loop_flat_sizes.size() + 1 + f_index) {
			for (int s_index = 0; s_index < this->post_loop_flat_sizes[f_index]; s_index++) {
				this->post_loop_flat_inputs[f_index]->acti_vals[s_index] = 0.0;
			}
		} else {
			for (int s_index = 0; s_index < this->post_loop_flat_sizes[f_index]; s_index++) {
				this->post_loop_flat_inputs[f_index]->acti_vals[s_index] = post_loop_flat_vals[f_index][s_index];
			}
		}
	}

	for (int sc_index = 0; sc_index < (int)outer_state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)outer_state_vals[sc_index].size(); st_index++) {
			this->outer_state_inputs[sc_index]->acti_vals[st_index] = outer_state_vals[sc_index][st_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldCombineNetwork::outer_activate(vector<double>& loop_state,
										vector<vector<double>>& post_loop_flat_vals,
										vector<vector<double>>& outer_state_vals) {
	if (this->outer_fold_index >= (int)this->pre_loop_flat_sizes.size()) {
		for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
			this->loop_state_input->acti_vals[s_index] = 0.0;
		}
	} else {
		for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
			this->loop_state_input->acti_vals[s_index] = loop_state[s_index];
		}
	}

	for (int f_index = 0; f_index < (int)this->post_loop_flat_sizes.size(); f_index++) {
		if (this->outer_fold_index >= (int)this->pre_loop_flat_sizes.size() + 1 + f_index) {
			for (int s_index = 0; s_index < this->post_loop_flat_sizes[f_index]; s_index++) {
				this->post_loop_flat_inputs[f_index]->acti_vals[s_index] = 0.0;
			}
		} else {
			for (int s_index = 0; s_index < this->post_loop_flat_sizes[f_index]; s_index++) {
				this->post_loop_flat_inputs[f_index]->acti_vals[s_index] = post_loop_flat_vals[f_index][s_index];
			}
		}
	}

	for (int sc_index = 0; sc_index < (int)outer_state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)outer_state_vals[sc_index].size(); st_index++) {
			this->outer_state_inputs[sc_index]->acti_vals[st_index] = outer_state_vals[sc_index][st_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldCombineNetwork::outer_backprop_last_state(vector<double>& errors,
												   double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}
	this->output->backprop();

	this->hidden->fold_loop_backprop_last_state();

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

void FoldCombineNetwork::outer_backprop_full_state(vector<double>& errors,
												   double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}
	this->output->backprop();

	this->hidden->fold_loop_backprop_full_state((int)this->outer_scope_sizes.size());

	this->epoch_iter++;
	if (this->epoch_iter == 100) {
		double hidden_max_update = 0.0;
		this->hidden->fold_get_max_update_full_state((int)this->outer_scope_sizes.size(),
													 hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
		if (hidden_learning_rate*hidden_max_update > target_max_update) {
			hidden_learning_rate = target_max_update/hidden_max_update;
		}
		this->hidden->fold_update_weights_full_state((int)this->outer_scope_sizes.size(),
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

void FoldCombineNetwork::save(ofstream& output_file) {
	output_file << this->loop_state_size << endl;
	output_file << this->pre_loop_flat_sizes.size() << endl;
	for (int f_index = 0; f_index < (int)this->pre_loop_flat_sizes.size(); f_index++) {
		output_file << this->pre_loop_flat_sizes[f_index] << endl;
	}
	output_file << this->post_loop_flat_sizes.size() << endl;
	for (int f_index = 0; f_index < (int)this->post_loop_flat_sizes.size(); f_index++) {
		output_file << this->post_loop_flat_sizes[f_index] << endl;
	}

	output_file << this->outer_fold_index << endl;
	output_file << this->average_error << endl;

	output_file << this->outer_scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->outer_scope_sizes.size(); sc_index++) {
		output_file << this->outer_scope_sizes[sc_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}
