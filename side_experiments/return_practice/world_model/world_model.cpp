#include "world_model.h"

#include "network.h"

using namespace std;

WorldModel::WorldModel() {
	this->num_states = 0;

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

WorldModel::WorldModel(WorldModel* original) {
	this->num_states = original->num_states;

	this->obs_network_inputs = original->obs_network_inputs;
	this->obs_network_outputs = original->obs_network_outputs;
	for (int n_index = 0; n_index < (int)original->obs_networks.size(); n_index++) {
		this->obs_networks.push_back(new Network(original->obs_networks[n_index]));
	}

	this->action_network_inputs = original->action_network_inputs;
	this->action_network_outputs = original->action_network_outputs;
	for (int n_index = 0; n_index < (int)original->action_networks.size(); n_index++) {
		this->action_networks.push_back(new Network(original->action_networks[n_index]));
	}

	this->final_network_inputs = original->final_network_inputs;
	for (int n_index = 0; n_index < (int)original->final_networks.size(); n_index++) {
		this->final_networks.push_back(new Network(original->final_networks[n_index]));
	}

	this->epoch_iter = 0;
	this->average_max_update = original->average_max_update;
}

WorldModel::~WorldModel() {
	for (int n_index = 0; n_index < (int)this->obs_networks.size(); n_index++) {
		delete this->obs_networks[n_index];
	}

	for (int n_index = 0; n_index < (int)this->action_networks.size(); n_index++) {
		delete this->action_networks[n_index];
	}

	for (int n_index = 0; n_index < (int)this->final_networks.size(); n_index++) {
		delete this->final_networks[n_index];
	}
}

void WorldModel::save(ofstream& output_file) {
	output_file << this->num_states << endl;

	output_file << this->obs_networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->obs_networks.size(); n_index++) {
		output_file << this->obs_network_inputs[n_index].size() << endl;
		for (int i_index = 0; i_index < (int)this->obs_network_inputs[n_index].size(); i_index++) {
			output_file << this->obs_network_inputs[n_index][i_index] << endl;
		}

		output_file << this->obs_network_outputs[n_index].size() << endl;
		for (int o_index = 0; o_index < (int)this->obs_network_outputs[n_index].size(); o_index++) {
			output_file << this->obs_network_outputs[n_index][o_index] << endl;
		}

		this->obs_networks[n_index]->save(output_file);
	}

	output_file << this->action_networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->action_networks.size(); n_index++) {
		output_file << this->action_network_inputs[n_index].size() << endl;
		for (int i_index = 0; i_index < (int)this->action_network_inputs[n_index].size(); i_index++) {
			output_file << this->action_network_inputs[n_index][i_index] << endl;
		}

		output_file << this->action_network_outputs[n_index].size() << endl;
		for (int o_index = 0; o_index < (int)this->action_network_outputs[n_index].size(); o_index++) {
			output_file << this->action_network_outputs[n_index][o_index] << endl;
		}

		this->action_networks[n_index]->save(output_file);
	}

	output_file << this->final_networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->final_networks.size(); n_index++) {
		output_file << this->final_network_inputs[n_index].size() << endl;
		for (int i_index = 0; i_index < (int)this->final_network_inputs[n_index].size(); i_index++) {
			output_file << this->final_network_inputs[n_index][i_index] << endl;
		}

		this->final_networks[n_index]->save(output_file);
	}

	output_file << this->average_max_update << endl;
}

void WorldModel::load(ifstream& input_file) {
	string num_state_line;
	getline(input_file, num_state_line);
	this->num_states = stoi(num_state_line);

	string num_obs_networks_line;
	getline(input_file, num_obs_networks_line);
	int num_obs_networks = stoi(num_obs_networks_line);
	for (int n_index = 0; n_index < num_obs_networks; n_index++) {
		string num_inputs_line;
		getline(input_file, num_inputs_line);
		int num_inputs = stoi(num_inputs_line);
		vector<int> inputs;
		for (int i_index = 0; i_index < num_inputs; i_index++) {
			string input_line;
			getline(input_file, input_line);
			inputs.push_back(stoi(input_line));
		}
		this->obs_network_inputs.push_back(inputs);

		string num_outputs_line;
		getline(input_file, num_outputs_line);
		int num_outputs = stoi(num_outputs_line);
		vector<int> outputs;
		for (int o_index = 0; o_index < num_outputs; o_index++) {
			string output_line;
			getline(input_file, output_line);
			outputs.push_back(stoi(output_line));
		}
		this->obs_network_outputs.push_back(outputs);

		this->obs_networks.push_back(new Network(input_file));
	}

	string num_action_networks_line;
	getline(input_file, num_action_networks_line);
	int num_action_networks = stoi(num_action_networks_line);
	for (int n_index = 0; n_index < num_action_networks; n_index++) {
		string num_inputs_line;
		getline(input_file, num_inputs_line);
		int num_inputs = stoi(num_inputs_line);
		vector<int> inputs;
		for (int i_index = 0; i_index < num_inputs; i_index++) {
			string input_line;
			getline(input_file, input_line);
			inputs.push_back(stoi(input_line));
		}
		this->action_network_inputs.push_back(inputs);

		string num_outputs_line;
		getline(input_file, num_outputs_line);
		int num_outputs = stoi(num_outputs_line);
		vector<int> outputs;
		for (int o_index = 0; o_index < num_outputs; o_index++) {
			string output_line;
			getline(input_file, output_line);
			outputs.push_back(stoi(output_line));
		}
		this->action_network_outputs.push_back(outputs);

		this->action_networks.push_back(new Network(input_file));
	}

	string num_final_networks_line;
	getline(input_file, num_final_networks_line);
	int num_final_networks = stoi(num_final_networks_line);
	for (int n_index = 0; n_index < num_final_networks; n_index++) {
		string num_inputs_line;
		getline(input_file, num_inputs_line);
		int num_inputs = stoi(num_inputs_line);
		vector<int> inputs;
		for (int i_index = 0; i_index < num_inputs; i_index++) {
			string input_line;
			getline(input_file, input_line);
			inputs.push_back(stoi(input_line));
		}
		this->final_network_inputs.push_back(inputs);

		this->final_networks.push_back(new Network(input_file));
	}

	string average_max_update_line;
	getline(input_file, average_max_update_line);
	this->average_max_update = stod(average_max_update_line);
}
