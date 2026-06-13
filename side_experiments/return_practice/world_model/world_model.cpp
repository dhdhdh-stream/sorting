#include "world_model.h"

#include "constants.h"
#include "network.h"
#include "predict_wrapper.h"
#include "state_network.h"

using namespace std;

WorldModel::WorldModel(int num_obs,
					   int num_actions) {
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
	this->obs_networks.push_back(new StateNetwork(STARTING_NUM_STATE + num_obs, STARTING_NUM_STATE));
	this->action_networks.push_back(new StateNetwork(STARTING_NUM_STATE + num_actions, STARTING_NUM_STATE));

	this->curr_final_network = new StateNetwork(STARTING_NUM_STATE, 1);

	this->curr_epoch_iter = 0;
	this->curr_average_max_update = 0.0;

	this->curr_misguess_average = 0.0;
	this->curr_misguess_variance_average = 0.0;

	this->curr_predict = new PredictWrapper();

	this->curr_candidate_predict = new PredictWrapper();
	this->curr_candidate_iter = 0;

	this->large_obs_network = new StateNetwork(STARTING_NUM_STATE + NUM_STATE_CHANGE + num_obs, NUM_STATE_CHANGE);
	this->large_obs_network->resize(STARTING_NUM_STATE + NUM_STATE_CHANGE);
	this->large_action_network = new StateNetwork(STARTING_NUM_STATE + NUM_STATE_CHANGE + num_actions, NUM_STATE_CHANGE);
	this->large_action_network->resize(STARTING_NUM_STATE + NUM_STATE_CHANGE);

	this->large_final_network = new StateNetwork(STARTING_NUM_STATE + NUM_STATE_CHANGE, 1);
	this->large_final_network->resize(STARTING_NUM_STATE + NUM_STATE_CHANGE);

	this->large_epoch_iter = 0;
	this->large_average_max_update = 0.0;

	this->large_misguess_average = 0.0;
	this->large_misguess_variance_average = 0.0;

	this->large_predict = new PredictWrapper();
	this->large_predict->add_states();

	this->large_candidate_predict = new PredictWrapper();
	this->large_candidate_predict->add_states();
	this->large_candidate_iter = 0;
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

	this->curr_final_network = new StateNetwork(input_file);

	this->curr_epoch_iter = 0;

	string curr_average_max_update_line;
	getline(input_file, curr_average_max_update_line);
	this->curr_average_max_update = stod(curr_average_max_update_line);

	string curr_misguess_average_line;
	getline(input_file, curr_misguess_average_line);
	this->curr_misguess_average = stod(curr_misguess_average_line);

	string curr_misguess_variance_average_line;
	getline(input_file, curr_misguess_variance_average_line);
	this->curr_misguess_variance_average = stod(curr_misguess_variance_average_line);

	string curr_history_size_line;
	getline(input_file, curr_history_size_line);
	int curr_history_size = stoi(curr_history_size_line);
	for (int h_index = 0; h_index < curr_history_size; h_index++) {
		string misguess_average_line;
		getline(input_file, misguess_average_line);
		this->curr_misguess_average_history.push_back(stod(misguess_average_line));

		string misguess_variance_average_line;
		getline(input_file, misguess_variance_average_line);
		this->curr_misguess_variance_average_history.push_back(stod(misguess_variance_average_line));
	}

	this->curr_predict = new PredictWrapper(input_file);

	this->curr_candidate_predict = new PredictWrapper(input_file);

	string curr_candidate_iter_line;
	getline(input_file, curr_candidate_iter_line);
	this->curr_candidate_iter = stoi(curr_candidate_iter_line);

	this->large_obs_network = new StateNetwork(input_file);
	this->large_action_network = new StateNetwork(input_file);

	this->large_final_network = new StateNetwork(input_file);

	this->large_epoch_iter = 0;

	string large_average_max_update_line;
	getline(input_file, large_average_max_update_line);
	this->large_average_max_update = stod(large_average_max_update_line);

	string large_misguess_average_line;
	getline(input_file, large_misguess_average_line);
	this->large_misguess_average = stod(large_misguess_average_line);

	string large_misguess_variance_average_line;
	getline(input_file, large_misguess_variance_average_line);
	this->large_misguess_variance_average = stod(large_misguess_variance_average_line);

	string large_history_size_line;
	getline(input_file, large_history_size_line);
	int large_history_size = stoi(large_history_size_line);
	for (int h_index = 0; h_index < large_history_size; h_index++) {
		string misguess_average_line;
		getline(input_file, misguess_average_line);
		this->large_misguess_average_history.push_back(stod(misguess_average_line));

		string misguess_variance_average_line;
		getline(input_file, misguess_variance_average_line);
		this->large_misguess_variance_average_history.push_back(stod(misguess_variance_average_line));
	}

	this->large_predict = new PredictWrapper(input_file);

	this->large_candidate_predict = new PredictWrapper(input_file);

	string large_candidate_iter_line;
	getline(input_file, large_candidate_iter_line);
	this->large_candidate_iter = stoi(large_candidate_iter_line);
}

WorldModel::~WorldModel() {
	for (int n_index = 0; n_index < (int)this->obs_networks.size(); n_index++) {
		delete this->obs_networks[n_index];
	}
	for (int n_index = 0; n_index < (int)this->action_networks.size(); n_index++) {
		delete this->action_networks[n_index];
	}

	delete this->curr_final_network;

	delete this->curr_predict;

	delete this->curr_candidate_predict;

	delete this->large_obs_network;
	delete this->large_action_network;

	delete this->large_final_network;

	delete this->large_predict;

	delete this->large_candidate_predict;
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

	this->curr_final_network->save(output_file);

	output_file << this->curr_average_max_update << endl;

	output_file << this->curr_misguess_average << endl;
	output_file << this->curr_misguess_variance_average << endl;

	output_file << this->curr_misguess_average_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->curr_misguess_average_history.size(); h_index++) {
		output_file << this->curr_misguess_average_history[h_index] << endl;
		output_file << this->curr_misguess_variance_average_history[h_index] << endl;
	}

	this->curr_predict->save(output_file);

	this->curr_candidate_predict->save(output_file);
	output_file << this->curr_candidate_iter << endl;

	this->large_obs_network->save(output_file);
	this->large_action_network->save(output_file);

	this->large_final_network->save(output_file);

	output_file << this->large_average_max_update << endl;

	output_file << this->large_misguess_average << endl;
	output_file << this->large_misguess_variance_average << endl;

	output_file << this->large_misguess_average_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->large_misguess_average_history.size(); h_index++) {
		output_file << this->large_misguess_average_history[h_index] << endl;
		output_file << this->large_misguess_variance_average_history[h_index] << endl;
	}

	this->large_predict->save(output_file);

	this->large_candidate_predict->save(output_file);
	output_file << this->large_candidate_iter << endl;
}
