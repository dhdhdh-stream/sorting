#include "network.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 20;

Network::Network(int input_size) {
	Layer* input_layer = new Layer(LINEAR_LAYER, input_size);
	this->inputs.push_back(input_layer);

	Layer* hidden_layer = new Layer(LEAKY_LAYER, NETWORK_INCREMENT_HIDDEN_SIZE);
	this->hiddens.push_back(hidden_layer);

	this->hidden_inputs.push_back(vector<pair<bool,int>>{{true, 0}});
	hidden_layer->input_layers.push_back(input_layer);
	hidden_layer->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);

	this->output_inputs.push_back(0);
	this->output->input_layers.push_back(hidden_layer);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_updates = vector<double>{0.0};
	this->output_average_max_update = 0.0;
}

Network::Network(Network* original) {
	for (int i_index = 0; i_index < (int)original->inputs.size(); i_index++) {
		Layer* input_layer = new Layer(LINEAR_LAYER, (int)original->inputs[i_index]->acti_vals.size());
		this->inputs.push_back(input_layer);
	}

	for (int h_index = 0; h_index < (int)original->hiddens.size(); h_index++) {
		Layer* hidden_layer = new Layer(LEAKY_LAYER, (int)original->hiddens[h_index]->acti_vals.size());
		this->hiddens.push_back(hidden_layer);
	}

	this->hidden_inputs = original->hidden_inputs;
	for (int h_index = 0; h_index < (int)this->hidden_inputs.size(); h_index++) {
		for (int i_index = 0; i_index < (int)this->hidden_inputs[h_index].size(); i_index++) {
			if (this->hidden_inputs[h_index][i_index].first) {
				this->hiddens[h_index]->input_layers.push_back(
					this->inputs[this->hidden_inputs[h_index][i_index].second]);
			} else {
				this->hiddens[h_index]->input_layers.push_back(
					this->hiddens[this->hidden_inputs[h_index][i_index].second]);
			}
		}
		this->hiddens[h_index]->setup_weights_full();
		this->hiddens[h_index]->copy_weights_from(original->hiddens[h_index]);
	}

	this->output = new Layer(LINEAR_LAYER, 1);

	this->output_inputs = original->output_inputs;
	for (int i_index = 0; i_index < (int)this->output_inputs.size(); i_index++) {
		this->output->input_layers.push_back(this->hiddens[this->output_inputs[i_index]]);
	}
	this->output->setup_weights_full();
	this->output->copy_weights_from(original->output);

	this->epoch_iter = 0;
	this->hidden_average_max_updates = vector<double>(this->hiddens.size(), 0.0);
	this->output_average_max_update = 0.0;
}

Network::Network(ifstream& input_file) {
	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string input_size_line;
		getline(input_file, input_size_line);
		Layer* input_layer = new Layer(LINEAR_LAYER, stoi(input_size_line));
		this->inputs.push_back(input_layer);
	}

	string num_hiddens_line;
	getline(input_file, num_hiddens_line);
	int num_hiddens = stoi(num_hiddens_line);
	for (int h_index = 0; h_index < num_hiddens; h_index++) {
		string hidden_size_line;
		getline(input_file, hidden_size_line);
		Layer* hidden_layer = new Layer(LEAKY_LAYER, stoi(hidden_size_line));
		this->hiddens.push_back(hidden_layer);
	}

	for (int h_index = 0; h_index < num_hiddens; h_index++) {
		string hidden_inputs_size_line;
		getline(input_file, hidden_inputs_size_line);
		int hidden_inputs_size = stoi(hidden_inputs_size_line);

		vector<pair<bool,int>> curr_hidden_inputs;
		for (int i_index = 0; i_index < hidden_inputs_size; i_index++) {
			string is_input_line;
			getline(input_file, is_input_line);

			string index_line;
			getline(input_file, index_line);

			curr_hidden_inputs.push_back({stoi(is_input_line), stoi(index_line)});
		}
		this->hidden_inputs.push_back(curr_hidden_inputs);
	}
	for (int h_index = 0; h_index < num_hiddens; h_index++) {
		for (int i_index = 0; i_index < (int)this->hidden_inputs[h_index].size(); i_index++) {
			if (this->hidden_inputs[h_index][i_index].first) {
				this->hiddens[h_index]->input_layers.push_back(
					this->inputs[this->hidden_inputs[h_index][i_index].second]);
			} else {
				this->hiddens[h_index]->input_layers.push_back(
					this->hiddens[this->hidden_inputs[h_index][i_index].second]);
			}
		}
		this->hiddens[h_index]->setup_weights_full();
	}

	string output_inputs_size_line;
	getline(input_file, output_inputs_size_line);
	int output_inputs_size = stoi(output_inputs_size_line);
	for (int i_index = 0; i_index < output_inputs_size; i_index++) {
		string index_line;
		getline(input_file, index_line);
		this->output_inputs.push_back(stoi(index_line));
	}
	this->output = new Layer(LINEAR_LAYER, 1);
	for (int i_index = 0; i_index < (int)this->output_inputs.size(); i_index++) {
		this->output->input_layers.push_back(this->hiddens[this->output_inputs[i_index]]);
	}
	this->output->setup_weights_full();

	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		this->hiddens[h_index]->load_weights_from(input_file);
	}
	this->output->load_weights_from(input_file);

	this->epoch_iter = 0;
	this->hidden_average_max_updates = vector<double>(this->hiddens.size(), 0.0);
	this->output_average_max_update = 0.0;
}

Network::~Network() {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		delete this->inputs[i_index];
	}

	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		delete this->hiddens[h_index];
	}

	delete this->output;
}

void Network::activate(vector<vector<double>>& input_vals) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		for (int v_index = 0; v_index < (int)input_vals[i_index].size(); v_index++) {
			this->inputs[i_index]->acti_vals[v_index] = input_vals[i_index][v_index];
		}
	}

	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		this->hiddens[h_index]->activate();
	}

	this->output->activate();
}

void Network::backprop(double error) {
	this->output->errors[0] = error;
	this->output->backprop();

	for (int h_index = (int)this->hiddens.size()-1; h_index >= 0; h_index--) {
		this->hiddens[h_index]->backprop();
	}

	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
			double hidden_max_update = 0.0;
			this->hiddens[h_index]->get_max_update(hidden_max_update);
			this->hidden_average_max_updates[h_index] = 0.999*this->hidden_average_max_updates[h_index]+0.001*hidden_max_update;
			if (hidden_max_update > 0.0) {
				double hidden_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_average_max_updates[h_index];
				if (hidden_learning_rate*hidden_max_update > NETWORK_TARGET_MAX_UPDATE) {
					hidden_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_max_update;
				}
				this->hiddens[h_index]->update_weights(hidden_learning_rate);
			}
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > NETWORK_TARGET_MAX_UPDATE) {
				output_learning_rate = NETWORK_TARGET_MAX_UPDATE/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void Network::increment_side(int input_size) {
	Layer* input_layer = new Layer(LINEAR_LAYER, input_size);
	this->inputs.push_back(input_layer);

	Layer* hidden_layer = new Layer(LEAKY_LAYER, NETWORK_INCREMENT_HIDDEN_SIZE);
	this->hiddens.push_back(hidden_layer);

	this->hidden_inputs.push_back(vector<pair<bool,int>>{{true, this->inputs.size()-1}});
	hidden_layer->input_layers.push_back(input_layer);
	hidden_layer->setup_weights_full();

	this->output_inputs.push_back((int)this->hidden_inputs.size()-1);
	this->output->add_input(hidden_layer);

	this->hidden_average_max_updates.push_back(0.0);
}

void Network::increment_above(int input_size) {
	Layer* input_layer = new Layer(LINEAR_LAYER, input_size);
	this->inputs.push_back(input_layer);

	Layer* hidden_layer = new Layer(LEAKY_LAYER, NETWORK_INCREMENT_HIDDEN_SIZE);
	this->hiddens.push_back(hidden_layer);

	vector<pair<bool,int>> new_hidden_inputs;
	for (int i_index = 0; i_index < (int)this->output_inputs.size(); i_index++) {
		new_hidden_inputs.push_back({false, this->output_inputs[i_index]});
	}
	new_hidden_inputs.push_back({true, this->inputs.size()-1});
	this->hidden_inputs.push_back(new_hidden_inputs);
	for (int i_index = 0; i_index < (int)new_hidden_inputs.size(); i_index++) {
		if (new_hidden_inputs[i_index].first) {
			hidden_layer->input_layers.push_back(
				this->inputs[new_hidden_inputs[i_index].second]);
		} else {
			hidden_layer->input_layers.push_back(
				this->hiddens[new_hidden_inputs[i_index].second]);
		}
	}
	hidden_layer->setup_weights_full();

	for (int i_index = 0; i_index < (int)this->output->weights[0].size(); i_index++) {
		for (int s_index = 0; s_index < (int)this->output->weights[0][i_index].size(); s_index++) {
			hidden_layer->weights[0][i_index][s_index] = this->output->weights[0][i_index][s_index];
			hidden_layer->constants[0] = this->output->constants[0];
			hidden_layer->weights[1][i_index][s_index] = -this->output->weights[0][i_index][s_index];
			hidden_layer->constants[1] = -this->output->constants[0];
		}
	}

	delete this->output;
	this->output = new Layer(LINEAR_LAYER, 1);

	this->output_inputs = vector<int>{(int)this->hiddens.size()-1};
	this->output->input_layers.push_back(hidden_layer);
	this->output->setup_weights_full();

	this->output->weights[0][0][0] = 1.0;
	this->output->weights[0][0][1] = -1.0;

	this->hidden_average_max_updates.push_back(0.0);
	this->output_average_max_update = 0.0;
}

void Network::save(ofstream& output_file) {
	output_file << this->inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		output_file << this->inputs[i_index]->acti_vals.size() << endl;
	}

	output_file << this->hiddens.size() << endl;
	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		output_file << this->hiddens[h_index]->acti_vals.size() << endl;
	}

	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		output_file << this->hidden_inputs[h_index].size() << endl;
		for (int i_index = 0; i_index < (int)this->hidden_inputs[h_index].size(); i_index++) {
			output_file << this->hidden_inputs[h_index][i_index].first << endl;
			output_file << this->hidden_inputs[h_index][i_index].second << endl;
		}
	}

	output_file << this->output_inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->output_inputs.size(); i_index++) {
		output_file << this->output_inputs[i_index] << endl;
	}

	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		this->hiddens[h_index]->save_weights(output_file);
	}
	this->output->save_weights(output_file);
}
