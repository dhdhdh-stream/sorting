#include "build_network.h"

#include <cmath>

#include "build_node.h"
#include "globals.h"
#include "layer.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 20;

const int SAMPLES_PER_ITER = 10;

BuildNetwork::BuildNetwork(int num_inputs) {
	this->inputs = vector<double>(num_inputs);

	this->output_constant = 0.0;
	this->output_constant_update = 0.0;

	this->epoch_iter = 0;
	this->average_max_update = 0.0;

	this->history_index = 0;
}

BuildNetwork::~BuildNetwork() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}
}

double BuildNetwork::activate(vector<double>& obs) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		this->inputs[i_index] = obs[i_index];
	}

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->activate(this);
	}

	double sum_vals = this->output_constant;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		sum_vals += this->output_weights[n_index] * this->nodes[n_index]->output->acti_vals[0];
	}

	return sum_vals;
}

void BuildNetwork::backprop(std::vector<double>& obs,
							double target_val) {
	if (this->obs_histories.size() < BUILD_NUM_TOTAL_SAMPLES) {
		this->obs_histories.push_back(obs);
		this->target_val_histories.push_back(target_val);

		if (this->obs_histories.size() >= BUILD_NUM_TOTAL_SAMPLES) {
			update_helper();
		}
	} else {
		this->obs_histories[this->history_index] = obs;
		this->target_val_histories[this->history_index] = target_val;

		this->history_index++;
		if (this->history_index >= BUILD_NUM_TOTAL_SAMPLES) {
			this->history_index = 0;

			update_helper();
		}

		uniform_int_distribution<int> sample_distribution(0, this->obs_histories.size()-1);
		for (int s_index = 0; s_index < SAMPLES_PER_ITER; s_index++) {
			int sample_index = sample_distribution(generator);

			double val = activate(this->obs_histories[sample_index]);

			double error = this->target_val_histories[sample_index] - val;

			for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
				this->nodes[n_index]->output->errors[0] += error * this->output_weights[n_index];

				this->output_weight_updates[n_index] += error * this->nodes[n_index]->output->acti_vals[0];
			}
			this->output_constant_update += error;

			for (int n_index = (int)this->nodes.size()-1; n_index >= 0; n_index--) {
				this->nodes[n_index]->backprop(this);
			}

			this->epoch_iter++;
			if (this->epoch_iter == EPOCH_SIZE) {
				double max_update = 0.0;
				for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
					double update_size = abs(this->output_weight_updates[n_index]);
					if (update_size > max_update) {
						max_update = update_size;
					}
				}
				{
					double update_size = abs(this->output_constant_update);
					if (update_size > max_update) {
						max_update = update_size;
					}
				}

				for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
					this->nodes[n_index]->get_max_update(max_update);
				}

				this->average_max_update = 0.999*this->average_max_update + 0.001*max_update;
				if (max_update > 0.0) {
					double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->average_max_update;
					if (learning_rate * max_update > NETWORK_TARGET_MAX_UPDATE) {
						learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
					}

					for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
						this->nodes[n_index]->update_weights(learning_rate);
					}

					for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
						double update = learning_rate * this->output_weight_updates[n_index];
						this->output_weight_updates[n_index] = 0.0;
						this->output_weights[n_index] += update;
					}
					{
						double update = learning_rate * this->output_constant_update;
						this->output_constant_update = 0.0;
						this->output_constant += update;
					}
				}
			}
		}
	}
}

void BuildNetwork::save(ofstream& output_file) {
	output_file << this->inputs.size() << endl;

	output_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->save(output_file);
	}

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		output_file << this->output_weights[n_index] << endl;
	}
	output_file << this->output_constant << endl;
}

void BuildNetwork::load(ifstream& input_file) {
	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		this->inputs.push_back(0.0);
	}

	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		BuildNode* build_node = new BuildNode(input_file);
		this->nodes.push_back(build_node);
	}

	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string weight_line;
		getline(input_file, weight_line);
		this->output_weights.push_back(stod(weight_line));
	}

	string output_constant_line;
	getline(input_file, output_constant_line);
	this->output_constant = stod(output_constant_line);

	for (int n_index = 0; n_index < num_nodes; n_index++) {
		this->output_weight_updates.push_back(0.0);
	}
}
