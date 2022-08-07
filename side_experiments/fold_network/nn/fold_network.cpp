#include "fold_network.h"

#include <iostream>

using namespace std;

void FoldNetwork::setup_layers() {
	this->global_input = new Layer(LINEAR_LAYER, this->global_size);

	this->fold_input = new Layer(LINEAR_LAYER, this->time_size);
	this->fold_state_input = new Layer(LINEAR_LAYER, this->fold_state_size);

	this->fold_layer = new Layer(RELU_LAYER, 100);
	this->fold_layer->input_layers.push_back(this->fold_input);
	this->fold_layer->input_layers.push_back(this->fold_state_input);
	this->fold_layer->input_layers.push_back(this->global_input);
	this->fold_layer->setup_weights_full();

	this->fold_output = new Layer(LINEAR_LAYER, this->fold_state_size);
	this->fold_output->input_layers.push_back(this->fold_layer);
	this->fold_output->setup_weights_full();

	if (this->state == FLAT) {
		this->process = new Layer(RELU_LAYER, 200);
		for (int i_index = 0; i_index < this->max_iters; i_index++) {
			this->process_inputs.push_back(new Layer(LINEAR_LAYER, this->time_size));
			this->process->input_layers.push_back(this->process_inputs[i_index]);
		}
	} else if (this->state == FOLD) {
		this->process = new Layer(RELU_LAYER, 200);
		this->process_inputs.push_back(new Layer(LINEAR_LAYER, this->fold_state_size));
		this->process->input_layers.push_back(this->process_inputs[0]);
		for (int i_index = this->index+1; i_index < this->max_iters; i_index++) {
			this->process_inputs.push_back(new Layer(LINEAR_LAYER, this->time_size));
			this->process->input_layers.push_back(this->process_inputs[i_index-this->index]);
		}
	} else {
		this->process = new Layer(RELU_LAYER, 20);
		this->process_inputs.push_back(new Layer(LINEAR_LAYER, this->fold_state_size));
		this->process->input_layers.push_back(this->process_inputs[0]);
	}
	this->process->input_layers.push_back(this->global_input);
	this->process->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->process);
	this->output->setup_weights_full();
}

FoldNetwork::FoldNetwork(int time_size,
						 int max_iters,
						 int global_size,
						 int fold_state_size) {
	this->time_size = time_size;
	this->max_iters = max_iters;
	this->global_size = global_size;

	this->fold_state_size = fold_state_size;

	this->state = FLAT;
	this->index = -1;

	setup_layers();

	this->epoch = 0;
	this->iter = 0;
}

FoldNetwork::FoldNetwork(FoldNetwork* original) {
	this->time_size = original->time_size;
	this->max_iters = original->max_iters;
	this->global_size = original->global_size;

	this->fold_state_size = original->fold_state_size;

	this->state = original->state;
	this->index = original->index;

	setup_layers();

	this->fold_layer->copy_weights_from(original->fold_layer);
	this->fold_output->copy_weights_from(original->fold_output);

	this->process->copy_weights_from(original->process);
	this->output->copy_weights_from(original->output);

	this->epoch = original->epoch;
	this->iter = 0;
}

FoldNetwork::FoldNetwork(ifstream& input_file) {
	string time_size_line;
	getline(input_file, time_size_line);
	this->time_size = stoi(time_size_line);

	string max_iters_line;
	getline(input_file, max_iters_line);
	this->max_iters = stoi(max_iters_line);

	string global_size_line;
	getline(input_file, global_size_line);
	this->global_size = stoi(global_size_line);

	string fold_state_size_line;
	getline(input_file, fold_state_size_line);
	this->fold_state_size = stoi(fold_state_size_line);

	string state_line;
	getline(input_file, state_line);
	this->state = stoi(state_line);

	string index_line;
	getline(input_file, index_line);
	this->index = stoi(index_line);

	setup_layers();

	this->fold_layer->load_weights_from(input_file);
	this->fold_output->load_weights_from(input_file);

	this->process->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	string epoch_line;
	getline(input_file, epoch_line);
	this->epoch = stoi(epoch_line);
	this->iter = 0;
}

FoldNetwork::~FoldNetwork() {
	delete this->global_input;

	delete this->fold_input;
	delete this->fold_state_input;
	delete this->fold_layer;
	delete this->fold_output;

	for (int i_index = 0; i_index < (int)this->process_inputs.size(); i_index++) {
		delete this->process_inputs[i_index];
	}
	delete this->process;
	delete this->output;
}

void FoldNetwork::train(int num_iterations,
						vector<vector<double>>& time_vals,
						vector<double>& global_vals) {
	for (int g_index = 0; g_index < this->global_size; g_index++) {
		this->global_input->acti_vals[g_index] = global_vals[g_index];
	}

	if (this->state == FLAT) {
		for (int i_index = 0; i_index < this->max_iters; i_index++) {
			if (i_index >= num_iterations) {
				for (int s_index = 0; s_index < this->time_size; s_index++) {
					this->process_inputs[i_index]->acti_vals[s_index] = 0.0;
				}
			} else {
				for (int s_index = 0; s_index < this->time_size; s_index++) {
					this->process_inputs[i_index]->acti_vals[s_index] = time_vals[i_index][s_index];
				}
			}
		}
	} else if (this->state == FOLD) {
		double fold_state[this->fold_state_size] = {};
		for (int i_index = 0; i_index < this->index+1; i_index++) {
			if (i_index >= num_iterations) {
				// do nothing
			} else {
				for (int s_index = 0; s_index < this->time_size; s_index++) {
					this->fold_input->acti_vals[s_index] = time_vals[i_index][s_index];
				}
				for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
					this->fold_state_input->acti_vals[f_index] = fold_state[f_index];
				}
				this->fold_layer->activate();
				this->fold_output->activate();
				push_fold_history();

				for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
					fold_state[f_index] = this->fold_output->acti_vals[f_index];
				}
			}
		}

		for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
			this->process_inputs[0]->acti_vals[f_index] = fold_state[f_index];
		}

		for (int i_index = this->index+1; i_index < this->max_iters; i_index++) {
			if (i_index >= num_iterations) {
				for (int s_index = 0; s_index < this->time_size; s_index++) {
					this->process_inputs[i_index-this->index]->acti_vals[s_index] = 0.0;
				}
			} else {
				for (int s_index = 0; s_index < this->time_size; s_index++) {
					this->process_inputs[i_index-this->index]->acti_vals[s_index] = time_vals[i_index][s_index];
				}
			}
		}
	} else {
		double fold_state[this->fold_state_size] = {};
		for (int i_index = 0; i_index < num_iterations; i_index++) {
			for (int s_index = 0; s_index < this->time_size; s_index++) {
				this->fold_input->acti_vals[s_index] = time_vals[i_index][s_index];
			}
			for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
				this->fold_state_input->acti_vals[f_index] = fold_state[f_index];
			}
			this->fold_layer->activate();
			this->fold_output->activate();

			for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
				fold_state[f_index] = this->fold_output->acti_vals[f_index];
			}
		}

		for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
			this->process_inputs[0]->acti_vals[f_index] = fold_state[f_index];
		}
	}
	
	this->process->activate();
	this->output->activate();
}

void FoldNetwork::backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->process->backprop();

	if (this->state == FLAT) {
		// do nothing
	} else if (this->state == FOLD) {
		double fold_state_errors[this->fold_state_size] = {};

		for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
			fold_state_errors[f_index] = this->process_inputs[0]->errors[f_index];
			this->process_inputs[0]->errors[f_index] = 0.0;
		}

		while (fold_layer_historys.size() > 0) {
			for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
				this->fold_output->errors[f_index] = fold_state_errors[f_index];
			}

			pop_fold_history();

			this->fold_output->backprop();
			this->fold_layer->backprop();

			for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
				fold_state_errors[f_index] = this->fold_state_input->errors[f_index];
				this->fold_state_input->errors[f_index] = 0.0;
			}
		}
	} else {
		// do nothing
	}
}

void FoldNetwork::increment() {
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

void FoldNetwork::calc_max_update(double& max_update,
								  double learning_rate,
								  double momentum) {
	if (this->state == FLAT) {
		this->process->calc_max_update(max_update,
									   learning_rate,
									   momentum);
		this->output->calc_max_update(max_update,
									  learning_rate,
									  momentum);
	} else if (this->state == FOLD) {
		this->process->collapse_calc_max_update(0,
												max_update,
												learning_rate,
												momentum);

		this->fold_layer->calc_max_update(
			max_update,
			learning_rate,
			momentum);
		this->fold_output->calc_max_update(
			max_update,
			learning_rate,
			momentum);
	} else {
		this->process->calc_max_update(max_update,
									   learning_rate,
									   momentum);
		this->output->calc_max_update(max_update,
									  learning_rate,
									  momentum);
	}
}

void FoldNetwork::update_weights(double factor,
								 double learning_rate,
								 double momentum) {
	if (this->state == FLAT) {
		this->process->update_weights(factor,
									  learning_rate,
									  momentum);
		this->output->update_weights(factor,
									 learning_rate,
									 momentum);
	} else if (this->state == FOLD) {
		this->process->collapse_update_weights(0,
											   factor,
											   learning_rate,
											   momentum);

		this->fold_layer->update_weights(
			factor,
			learning_rate,
			momentum);
		this->fold_output->update_weights(
			factor,
			learning_rate,
			momentum);
	} else {
		this->process->update_weights(factor,
									  learning_rate,
									  momentum);
		this->output->update_weights(factor,
									 learning_rate,
									 momentum);
	}
}

void FoldNetwork::next_step() {
	if (this->state == FLAT) {
		cout << "FLAT to FOLD" << endl;

		this->state = FOLD;
		this->index = 0;

		// first fold doesn't merge
		delete this->process_inputs[0];
		this->process_inputs[0] = new Layer(LINEAR_LAYER, this->fold_state_size);
		this->process->collapse_input(0, this->process_inputs[0]);
	} else if (this->state == FOLD) {
		if (this->index == this->max_iters-1) {
			cout << "FOLD to CLEAN" << endl;

			this->state = CLEAN;
			this->index = -1;

			delete this->process;
			delete this->output;
			this->process = new Layer(RELU_LAYER, 20);
			this->process->input_layers.push_back(this->process_inputs[0]);
			this->process->input_layers.push_back(this->global_input);
			this->process->setup_weights_full();
			this->output = new Layer(LINEAR_LAYER, 1);
			this->output->input_layers.push_back(this->process);
			this->output->setup_weights_full();
		} else {
			cout << "FOLD #" << this->index << " to FOLD #" << this->index+1 << endl;

			this->index++;

			delete this->process_inputs[0];
			this->process_inputs.erase(this->process_inputs.begin());
			delete this->process_inputs[0];
			this->process_inputs[0] = new Layer(LINEAR_LAYER, this->fold_state_size);
			this->process->fold_input(this->process_inputs[0]);
		}
	} else {
		// do nothing
	}
}

vector<double> FoldNetwork::fan(int num_iterations,
								vector<vector<double>>& time_vals,
								vector<double>& global_vals) {
	vector<double> fan_results;

	for (int g_index = 0; g_index < this->global_size; g_index++) {
		this->global_input->acti_vals[g_index] = global_vals[g_index];
	}

	double fold_state[this->fold_state_size] = {};
	for (int i_index = 0; i_index < num_iterations; i_index++) {
		for (int s_index = 0; s_index < this->time_size; s_index++) {
			this->fold_input->acti_vals[s_index] = time_vals[i_index][s_index];
		}
		for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
			this->fold_state_input->acti_vals[f_index] = fold_state[f_index];
		}
		this->fold_layer->activate();
		this->fold_output->activate();

		for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
			fold_state[f_index] = this->fold_output->acti_vals[f_index];
		}

		for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
			this->process_inputs[0]->acti_vals[f_index] = fold_state[f_index];
		}
		this->process->activate();
		this->output->activate();

		fan_results.push_back(this->output->acti_vals[0]);
	}

	return fan_results;
}

void FoldNetwork::save(ofstream& output_file) {
	output_file << this->time_size << endl;
	output_file << this->max_iters << endl;
	output_file << this->global_size << endl;

	output_file << this->fold_state_size << endl;

	output_file << this->state << endl;
	output_file << this->index << endl;

	this->fold_layer->save_weights(output_file);
	this->fold_output->save_weights(output_file);

	this->process->save_weights(output_file);
	this->output->save_weights(output_file);

	output_file << this->epoch << endl;
}

void FoldNetwork::push_fold_history() {
	vector<double> fold_input_history;
	for (int s_index = 0; s_index < this->time_size; s_index++) {
		fold_input_history.push_back(this->fold_input->acti_vals[s_index]);
	}
	this->fold_input_historys.push_back(fold_input_history);

	vector<double> fold_state_input_history;
	for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
		fold_state_input_history.push_back(this->fold_state_input->acti_vals[f_index]);
	}
	this->fold_state_input_historys.push_back(fold_state_input_history);

	vector<double> fold_layer_history;
	for (int n_index = 0; n_index < 100; n_index++) {
		fold_layer_history.push_back(this->fold_layer->acti_vals[n_index]);
	}
	this->fold_layer_historys.push_back(fold_layer_history);
}

void FoldNetwork::pop_fold_history() {
	vector<double> fold_input_history = this->fold_input_historys.back();
	for (int s_index = 0; s_index < this->time_size; s_index++) {
		this->fold_input->acti_vals[s_index] = fold_input_history[s_index];
	}
	this->fold_input_historys.pop_back();

	vector<double> fold_state_input_history = this->fold_state_input_historys.back();
	for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
		this->fold_state_input->acti_vals[f_index] = fold_state_input_history[f_index];
	}
	this->fold_state_input_historys.pop_back();

	vector<double> fold_layer_history = this->fold_layer_historys.back();
	for (int n_index = 0; n_index < 100; n_index++) {
		this->fold_layer->acti_vals[n_index] = fold_layer_history[n_index];
	}
	this->fold_layer_historys.pop_back();
}
