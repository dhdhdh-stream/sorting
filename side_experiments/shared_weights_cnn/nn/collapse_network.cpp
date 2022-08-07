#include "collapse_network.h"

#include <iostream>

using namespace std;

void CollapseNetwork::setup_layers() {
	this->global_input = new Layer(LINEAR_LAYER, this->global_size);

	this->collapse_input = new Layer(LINEAR_LAYER, this->time_size);

	this->collapse_layer = new Layer(RELU_LAYER, 50);
	this->collapse_layer->input_layers.push_back(this->collapse_input);
	this->collapse_layer->input_layers.push_back(this->global_input);
	this->collapse_layer->setup_weights_full();

	this->collapse_output = new Layer(LINEAR_LAYER, this->collapse_state_size);
	this->collapse_output->input_layers.push_back(this->collapse_layer);
	this->collapse_output->setup_weights_full();

	this->fold_collapse_input = new Layer(LINEAR_LAYER, this->collapse_state_size);
	this->fold_state_input = new Layer(LINEAR_LAYER, this->fold_state_size);

	this->fold_layer = new Layer(RELU_LAYER, 50);
	this->fold_layer->input_layers.push_back(this->fold_collapse_input);
	this->fold_layer->input_layers.push_back(this->fold_state_input);
	this->fold_layer->input_layers.push_back(this->global_input);
	this->fold_layer->setup_weights_full();

	this->fold_output = new Layer(LINEAR_LAYER, this->fold_state_size);
	this->fold_output->input_layers.push_back(this->fold_layer);
	this->fold_output->setup_weights_full();

	this->process = new Layer(RELU_LAYER, 200);
	if (this->state == FLAT) {
		for (int i_index = 0; i_index < this->max_iters; i_index++) {
			this->process_inputs.push_back(new Layer(LINEAR_LAYER, this->time_size));
			this->process->input_layers.push_back(this->process_inputs[i_index]);
		}
	} else if (this->state == COLLAPSE) {
		for (int i_index = 0; i_index < this->index+1; i_index++) {
			this->process_inputs.push_back(new Layer(LINEAR_LAYER, this->collapse_state_size));
			this->process->input_layers.push_back(this->process_inputs[i_index]);
		}
		for (int i_index = this->index+1; i_index < this->max_iters; i_index++) {
			this->process_inputs.push_back(new Layer(LINEAR_LAYER, this->time_size));
			this->process->input_layers.push_back(this->process_inputs[i_index]);
		}
	} else {
		this->process_inputs.push_back(new Layer(LINEAR_LAYER, this->fold_state_size));
		this->process->input_layers.push_back(this->process_inputs[0]);
		for (int i_index = this->index+1; i_index < this->max_iters; i_index++) {
			this->process_inputs.push_back(new Layer(LINEAR_LAYER, this->collapse_state_size));
			this->process->input_layers.push_back(this->process_inputs[i_index-this->index]);
		}
	}
	this->process->input_layers.push_back(this->global_input);
	this->process->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->process);
	this->output->setup_weights_full();
}

CollapseNetwork::CollapseNetwork(int time_size,
								 int max_iters,
								 int global_size,
								 int collapse_state_size,
								 int fold_state_size) {
	this->time_size = time_size;
	this->max_iters = max_iters;
	this->global_size = global_size;

	this->collapse_state_size = collapse_state_size;
	this->fold_state_size = fold_state_size;

	this->state = FLAT;
	this->index = -1;

	setup_layers();
}

CollapseNetwork::CollapseNetwork(CollapseNetwork* original) {
	this->time_size = original->time_size;
	this->max_iters = original->max_iters;
	this->global_size = original->global_size;

	this->collapse_state_size = original->collapse_state_size;
	this->fold_state_size = original->fold_state_size;

	this->state = original->state;
	this->index = original->index;

	setup_layers();

	this->collapse_layer->copy_weights_from(original->collapse_layer);
	this->collapse_output->copy_weights_from(original->collapse_output);
	
	this->fold_layer->copy_weights_from(original->fold_layer);
	this->fold_output->copy_weights_from(original->fold_output);

	this->process->copy_weights_from(original->process);
	this->output->copy_weights_from(original->output);
}

CollapseNetwork::CollapseNetwork(ifstream& input_file) {
	string time_size_line;
	getline(input_file, time_size_line);
	this->time_size = stoi(time_size_line);

	string max_iters_line;
	getline(input_file, max_iters_line);
	this->max_iters = stoi(max_iters_line);

	string global_size_line;
	getline(input_file, global_size_line);
	this->global_size = stoi(global_size_line);

	string collapse_state_size_line;
	getline(input_file, collapse_state_size_line);
	this->collapse_state_size = stoi(collapse_state_size_line);

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

	this->collapse_layer->load_weights_from(input_file);
	this->collapse_output->load_weights_from(input_file);

	this->fold_layer->load_weights_from(input_file);
	this->fold_output->load_weights_from(input_file);

	this->process->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

CollapseNetwork::~CollapseNetwork() {
	delete this->global_input;

	delete this->collapse_input;
	delete this->collapse_layer;
	delete this->collapse_output;

	delete this->fold_collapse_input;
	delete this->fold_state_input;
	delete this->fold_layer;
	delete this->fold_output;

	for (int i_index = 0; i_index < (int)this->process_inputs.size(); i_index++) {
		delete this->process_inputs[i_index];
	}
	delete this->process;
	delete this->output;
}

void CollapseNetwork::train(int num_iterations,
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
	} else if (this->state == COLLAPSE) {
		for (int i_index = 0; i_index < this->index+1; i_index++) {
			if (i_index >= num_iterations) {
				for (int c_index = 0; c_index < this->collapse_state_size; c_index++) {
					this->process_inputs[i_index]->acti_vals[c_index] = 0.0;
				}
			} else {
				for (int s_index = 0; s_index < this->time_size; s_index++) {
					this->collapse_input->acti_vals[s_index] = time_vals[i_index][s_index];
				}
				this->collapse_layer->activate();
				this->collapse_output->activate();
				push_collapse_history();

				for (int c_index = 0; c_index < this->collapse_state_size; c_index++) {
					this->process_inputs[i_index]->acti_vals[c_index] = this->collapse_output->acti_vals[c_index];
				}
			}
		}
		for (int i_index = this->index+1; i_index < this->max_iters; i_index++) {
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
	} else {
		double fold_state[this->fold_state_size] = {};
		for (int i_index = 0; i_index < this->index+1; i_index++) {
			if (i_index >= num_iterations) {
				// do nothing
			} else {
				for (int s_index = 0; s_index < this->time_size; s_index++) {
					this->collapse_input->acti_vals[s_index] = time_vals[i_index][s_index];
				}
				this->collapse_layer->activate();
				this->collapse_output->activate();

				for (int c_index = 0; c_index < this->collapse_state_size; c_index++) {
					this->fold_collapse_input->acti_vals[c_index] = this->collapse_output->acti_vals[c_index];
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
				for (int c_index = 0; c_index < this->collapse_state_size; c_index++) {
					this->process_inputs[i_index-this->index]->acti_vals[c_index] = 0.0;
				}
			} else {
				for (int s_index = 0; s_index < this->time_size; s_index++) {
					this->collapse_input->acti_vals[s_index] = time_vals[i_index][s_index];
				}
				this->collapse_layer->activate();
				this->collapse_output->activate();

				for (int c_index = 0; c_index < this->collapse_state_size; c_index++) {
					this->process_inputs[i_index-this->index]->acti_vals[c_index] = this->collapse_output->acti_vals[c_index];
				}
			}
		}
	}
	
	this->process->activate();
	this->output->activate();
}

void CollapseNetwork::backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->process->backprop();

	if (this->state == FLAT) {
		// do nothing
	} else if (this->state == COLLAPSE) {
		while (collapse_layer_historys.size() > 0) {
			int curr_time_index = (int)collapse_layer_historys.size()-1;

			for (int c_index = 0; c_index < this->collapse_state_size; c_index++) {
				this->collapse_output->errors[c_index] = this->process_inputs[curr_time_index]->errors[c_index];
				this->process_inputs[curr_time_index]->errors[c_index] = 0.0;
			}

			pop_collapse_history();

			this->collapse_output->backprop();
			this->collapse_layer->backprop();
		}
	} else {
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
	}
}

void CollapseNetwork::calc_max_update(double& max_update,
									  double learning_rate,
									  double momentum) {
	if (this->state == FLAT) {
		this->process->calc_max_update(max_update,
									   learning_rate,
									   momentum);
		this->output->calc_max_update(max_update,
									  learning_rate,
									  momentum);
	} else if (this->state == COLLAPSE) {
		this->process->collapse_calc_max_update(this->index,
												max_update,
												learning_rate,
												momentum);

		this->collapse_layer->calc_max_update(
			max_update,
			learning_rate,
			momentum);
		this->collapse_output->calc_max_update(
			max_update,
			learning_rate,
			momentum);
	} else {
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
	}
}

void CollapseNetwork::update_weights(double factor,
									 double learning_rate,
									 double momentum) {
	if (this->state == FLAT) {
		this->process->update_weights(factor,
									  learning_rate,
									  momentum);
		this->output->update_weights(factor,
									 learning_rate,
									 momentum);
	} else if (this->state == COLLAPSE) {
		this->process->collapse_update_weights(this->index,
											   factor,
											   learning_rate,
											   momentum);

		this->collapse_layer->update_weights(
			factor,
			learning_rate,
			momentum);
		this->collapse_output->update_weights(
			factor,
			learning_rate,
			momentum);
	} else {
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
	}
}

void CollapseNetwork::next_step() {
	if (this->state == FLAT) {
		cout << "FLAT to COLLAPSE" << endl;

		this->state = COLLAPSE;
		this->index = 0;

		delete this->process_inputs[0];
		this->process_inputs[0] = new Layer(LINEAR_LAYER, this->collapse_state_size);
		this->process->collapse_input(0, this->process_inputs[0]);
	} else if (this->state == COLLAPSE) {
		if (this->index == this->max_iters-1) {
			cout << "COLLAPSE to FOLD" << endl;

			this->state = FOLD;
			this->index = 0;

			// first fold doesn't merge
			delete this->process_inputs[0];
			this->process_inputs[0] = new Layer(LINEAR_LAYER, this->fold_state_size);
			this->process->collapse_input(0, this->process_inputs[0]);
		} else {
			cout << "COLLAPSE #" << this->index << " to COLLAPSE #" << this->index+1 << endl;

			this->index++;

			delete this->process_inputs[this->index];
			this->process_inputs[this->index] = new Layer(LINEAR_LAYER, this->collapse_state_size);
			this->process->collapse_input(this->index, this->process_inputs[this->index]);
		}
	} else {
		cout << "FOLD #" << this->index << " to FOLD #" << this->index+1 << endl;

		this->index++;

		delete this->process_inputs[0];
		this->process_inputs.erase(this->process_inputs.begin());
		delete this->process_inputs[0];
		this->process_inputs[0] = new Layer(LINEAR_LAYER, this->fold_state_size);
		this->process->fold_input(this->process_inputs[0]);
	}
}

vector<double> CollapseNetwork::fan(int num_iterations,
									vector<vector<double>>& time_vals,
									vector<double>& global_vals) {
	vector<double> fan_results;

	for (int g_index = 0; g_index < this->global_size; g_index++) {
		this->global_input->acti_vals[g_index] = global_vals[g_index];
	}

	double fold_state[this->fold_state_size] = {};
	for (int i_index = 0; i_index < num_iterations; i_index++) {
		for (int s_index = 0; s_index < this->time_size; s_index++) {
			this->collapse_input->acti_vals[s_index] = time_vals[i_index][s_index];
		}
		this->collapse_layer->activate();
		this->collapse_output->activate();

		for (int c_index = 0; c_index < this->collapse_state_size; c_index++) {
			this->fold_collapse_input->acti_vals[c_index] = this->collapse_output->acti_vals[c_index];
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

void CollapseNetwork::save(ofstream& output_file) {
	output_file << this->time_size << endl;
	output_file << this->max_iters << endl;
	output_file << this->global_size << endl;

	output_file << this->collapse_state_size << endl;
	output_file << this->fold_state_size << endl;

	output_file << this->state << endl;
	output_file << this->index << endl;

	this->collapse_layer->save_weights(output_file);
	this->collapse_output->save_weights(output_file);

	this->fold_layer->save_weights(output_file);
	this->fold_output->save_weights(output_file);

	this->process->save_weights(output_file);
	this->output->save_weights(output_file);
}

void CollapseNetwork::push_collapse_history() {
	vector<double> collapse_input_history;
	for (int s_index = 0; s_index < this->time_size; s_index++) {
		collapse_input_history.push_back(this->collapse_input->acti_vals[s_index]);
	}
	this->collapse_input_historys.push_back(collapse_input_history);

	vector<double> collapse_layer_history;
	for (int n_index = 0; n_index < 50; n_index++) {
		collapse_layer_history.push_back(this->collapse_layer->acti_vals[n_index]);
	}
	this->collapse_layer_historys.push_back(collapse_layer_history);
}

void CollapseNetwork::pop_collapse_history() {
	vector<double> collapse_input_history = this->collapse_input_historys.back();
	for (int s_index = 0; s_index < this->time_size; s_index++) {
		this->collapse_input->acti_vals[s_index] = collapse_input_history[s_index];
	}
	this->collapse_input_historys.pop_back();

	vector<double> collapse_layer_history = this->collapse_layer_historys.back();
	for (int n_index = 0; n_index < 50; n_index++) {
		this->collapse_layer->acti_vals[n_index] = collapse_layer_history[n_index];
	}
	this->collapse_layer_historys.pop_back();
}

void CollapseNetwork::push_fold_history() {
	vector<double> fold_collapse_input_history;
	for (int c_index = 0; c_index < this->collapse_state_size; c_index++) {
		fold_collapse_input_history.push_back(this->fold_collapse_input->acti_vals[c_index]);
	}
	this->fold_collapse_input_historys.push_back(fold_collapse_input_history);

	vector<double> fold_state_input_history;
	for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
		fold_state_input_history.push_back(this->fold_state_input->acti_vals[f_index]);
	}
	this->fold_state_input_historys.push_back(fold_state_input_history);

	vector<double> fold_layer_history;
	for (int n_index = 0; n_index < 50; n_index++) {
		fold_layer_history.push_back(this->fold_layer->acti_vals[n_index]);
	}
	this->fold_layer_historys.push_back(fold_layer_history);
}

void CollapseNetwork::pop_fold_history() {
	vector<double> fold_collapse_input_history = this->fold_collapse_input_historys.back();
	for (int c_index = 0; c_index < this->collapse_state_size; c_index++) {
		this->fold_collapse_input->acti_vals[c_index] = fold_collapse_input_history[c_index];
	}
	this->fold_collapse_input_historys.pop_back();

	vector<double> fold_state_input_history = this->fold_state_input_historys.back();
	for (int f_index = 0; f_index < this->fold_state_size; f_index++) {
		this->fold_state_input->acti_vals[f_index] = fold_state_input_history[f_index];
	}
	this->fold_state_input_historys.pop_back();

	vector<double> fold_layer_history = this->fold_layer_historys.back();
	for (int n_index = 0; n_index < 50; n_index++) {
		this->fold_layer->acti_vals[n_index] = fold_layer_history[n_index];
	}
	this->fold_layer_historys.pop_back();
}
