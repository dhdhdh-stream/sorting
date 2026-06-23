#include "world_model.h"

#include "constants.h"
#include "network.h"
#include "predict_wrapper.h"
#include "state_network.h"
#include "wrapper.h"

using namespace std;

WorldModel::WorldModel(Wrapper* wrapper) {
	this->num_states = STARTING_NUM_STATE;

	vector<int> inputs;
	for (int i_index = 0; i_index < STARTING_NUM_STATE; i_index++) {
		inputs.push_back(i_index);
	}
	this->network_inputs.push_back(inputs);
	vector<int> outputs;
	for (int o_index = 0; o_index < STARTING_NUM_STATE; o_index++) {
		outputs.push_back(o_index);
	}
	this->network_outputs.push_back(outputs);
	this->obs_networks.push_back(new StateNetwork(STARTING_NUM_STATE + wrapper->num_obs, STARTING_NUM_STATE));
	this->action_networks.push_back(new StateNetwork(STARTING_NUM_STATE + wrapper->num_actions, STARTING_NUM_STATE));

	this->final_network = new StateNetwork(STARTING_NUM_STATE, 1);

	this->epoch_iter = 0;
	this->average_max_update = 0.0;

	this->predict = new PredictWrapper(wrapper);

	this->candidate_predict = new PredictWrapper(wrapper);
	this->candidate_iter = 0;
}

WorldModel::WorldModel(ifstream& input_file) {
	string num_state_line;
	getline(input_file, num_state_line);
	this->num_states = stoi(num_state_line);

	string num_networks_line;
	getline(input_file, num_networks_line);
	int num_networks = stoi(num_networks_line);
	for (int n_index = 0; n_index < num_networks; n_index++) {
		vector<int> inputs;
		string num_inputs_line;
		getline(input_file, num_inputs_line);
		int num_inputs = stoi(num_inputs_line);
		for (int i_index = 0; i_index < num_inputs; i_index++) {
			string input_line;
			getline(input_file, input_line);
			inputs.push_back(stoi(input_line));
		}
		this->network_inputs.push_back(inputs);

		vector<int> outputs;
		string num_outputs_line;
		getline(input_file, num_outputs_line);
		int num_outputs = stoi(num_outputs_line);
		for (int o_index = 0; o_index < num_outputs; o_index++) {
			string output_line;
			getline(input_file, output_line);
			outputs.push_back(stoi(output_line));
		}
		this->network_outputs.push_back(outputs);

		this->obs_networks.push_back(new StateNetwork(input_file));
		this->action_networks.push_back(new StateNetwork(input_file));
	}

	this->final_network = new StateNetwork(input_file);

	this->epoch_iter = 0;

	string average_max_update_line;
	getline(input_file, average_max_update_line);
	this->average_max_update = stod(average_max_update_line);

	this->predict = new PredictWrapper(input_file);

	this->candidate_predict = new PredictWrapper(input_file);

	string candidate_iter_line;
	getline(input_file, candidate_iter_line);
	this->candidate_iter = stoi(candidate_iter_line);
}

WorldModel::~WorldModel() {
	for (int n_index = 0; n_index < (int)this->obs_networks.size(); n_index++) {
		delete this->obs_networks[n_index];
	}
	for (int n_index = 0; n_index < (int)this->action_networks.size(); n_index++) {
		delete this->action_networks[n_index];
	}

	delete this->final_network;

	delete this->predict;

	delete this->candidate_predict;
}

void WorldModel::save(ofstream& output_file) {
	output_file << this->num_states << endl;

	output_file << this->network_inputs.size() << endl;
	for (int n_index = 0; n_index < (int)this->network_inputs.size(); n_index++) {
		output_file << this->network_inputs[n_index].size() << endl;
		for (int i_index = 0; i_index < (int)this->network_inputs[n_index].size(); i_index++) {
			output_file << this->network_inputs[n_index][i_index] << endl;
		}

		output_file << this->network_outputs[n_index].size() << endl;
		for (int o_index = 0; o_index < (int)this->network_outputs[n_index].size(); o_index++) {
			output_file << this->network_outputs[n_index][o_index] << endl;
		}

		this->obs_networks[n_index]->save(output_file);
		this->action_networks[n_index]->save(output_file);
	}

	this->final_network->save(output_file);

	output_file << this->average_max_update << endl;

	this->predict->save(output_file);

	this->candidate_predict->save(output_file);
	output_file << this->candidate_iter << endl;
}
