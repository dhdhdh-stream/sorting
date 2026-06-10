#include "predict_wrapper.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "state_network.h"

using namespace std;

PredictWrapper::PredictWrapper() {
	this->average_network = new StateNetwork(STARTING_NUM_STATE, STARTING_NUM_STATE);
	this->average_epoch_iter = 0;
	this->average_average_max_update = 0.0;

	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		this->val_networks.push_back(new StateNetwork(STARTING_NUM_STATE, STARTING_NUM_STATE));
		this->val_epoch_iters.push_back(0);
		this->val_average_max_updates.push_back(0.0);

		this->select_networks.push_back(new Network(STARTING_NUM_STATE));
	}

	this->misguess_average = 0.0;
}

PredictWrapper::PredictWrapper(PredictWrapper* original) {
	this->average_network = new StateNetwork(original->average_network);
	this->average_epoch_iter = original->average_epoch_iter;
	this->average_average_max_update = original->average_average_max_update;

	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		this->val_networks.push_back(new StateNetwork(original->val_networks[n_index]));
		this->val_epoch_iters.push_back(original->val_epoch_iters[n_index]);
		this->val_average_max_updates.push_back(original->val_average_max_updates[n_index]);

		this->select_networks.push_back(new Network(original->select_networks[n_index]));
	}

	this->misguess_average = original->misguess_average;
}

PredictWrapper::PredictWrapper(ifstream& input_file) {
	this->average_network = new StateNetwork(input_file);
	this->average_epoch_iter = 0;

	string average_average_max_update_line;
	getline(input_file, average_average_max_update_line);
	this->average_average_max_update = stod(average_average_max_update_line);

	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		this->val_networks.push_back(new StateNetwork(input_file));
		this->val_epoch_iters.push_back(0);

		string average_max_update_line;
		getline(input_file, average_max_update_line);
		this->val_average_max_updates.push_back(stod(average_max_update_line));

		this->select_networks.push_back(new Network(input_file));
	}

	string misguess_average_line;
	getline(input_file, misguess_average_line);
	this->misguess_average = stod(misguess_average_line);
}

PredictWrapper::~PredictWrapper() {
	delete this->average_network;

	for (int n_index = 0; n_index < (int)this->val_networks.size(); n_index++) {
		delete this->val_networks[n_index];
	}

	for (int n_index = 0; n_index < (int)this->select_networks.size(); n_index++) {
		delete this->select_networks[n_index];
	}
}

void PredictWrapper::add_states() {
	this->average_network->add_inputs(NUM_STATE_CHANGE);
	this->average_network->add_outputs(NUM_STATE_CHANGE);

	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		this->val_networks[n_index]->add_inputs(NUM_STATE_CHANGE);
		this->val_networks[n_index]->add_outputs(NUM_STATE_CHANGE);

		this->select_networks[n_index]->add_inputs(NUM_STATE_CHANGE);
	}
}

void PredictWrapper::twiddle() {
	uniform_int_distribution<int> delete_distribution(0, this->val_networks.size()-1);
	int delete_index = delete_distribution(generator);
	delete this->val_networks[delete_index];
	this->val_networks.erase(this->val_networks.begin() + delete_index);
	this->val_epoch_iters.erase(this->val_epoch_iters.begin() + delete_index);
	this->val_average_max_updates.erase(this->val_average_max_updates.begin() + delete_index);
	delete this->select_networks[delete_index];
	this->select_networks.erase(this->select_networks.begin() + delete_index);

	this->val_networks.push_back(new StateNetwork(this->average_network));
	this->val_epoch_iters.push_back(0);
	this->val_average_max_updates.push_back(0.0);
	this->select_networks.push_back(new Network(this->select_networks[0]->input->acti_vals.size()));
}

void PredictWrapper::save(ofstream& output_file) {
	this->average_network->save(output_file);
	output_file << this->average_average_max_update << endl;

	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		this->val_networks[n_index]->save(output_file);
		output_file << this->val_average_max_updates[n_index] << endl;

		this->select_networks[n_index]->save(output_file);
	}

	output_file << this->misguess_average << endl;
}
