#include "predict_wrapper.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "state_network.h"

using namespace std;

PredictWrapper::PredictWrapper() {
	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		this->val_networks.push_back(new StateNetwork(STARTING_NUM_STATE, STARTING_NUM_STATE));
		this->val_epoch_iters.push_back(0);
		this->val_average_max_updates.push_back(0.0);

		this->select_networks.push_back(new Network(STARTING_NUM_STATE));
	}

	this->misguess_average = 0.0;
}

PredictWrapper::PredictWrapper(PredictWrapper* original) {
	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		this->val_networks.push_back(new StateNetwork(original->val_networks[n_index]));
		this->val_epoch_iters.push_back(original->val_epoch_iters[n_index]);
		this->val_average_max_updates.push_back(original->val_average_max_updates[n_index]);

		this->select_networks.push_back(new Network(original->select_networks[n_index]));
	}

	this->misguess_average = original->misguess_average;
}

PredictWrapper::PredictWrapper(ifstream& input_file) {
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
	for (int n_index = 0; n_index < (int)this->val_networks.size(); n_index++) {
		delete this->val_networks[n_index];
	}

	for (int n_index = 0; n_index < (int)this->select_networks.size(); n_index++) {
		delete this->select_networks[n_index];
	}
}

void PredictWrapper::add_states() {
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

	uniform_int_distribution<int> copy_distribution(0, this->val_networks.size()-1);
	int copy_index = copy_distribution(generator);
	this->val_networks.push_back(new StateNetwork(this->val_networks[copy_index]));
	this->val_networks.back()->twiddle();
	this->val_epoch_iters.push_back(this->val_epoch_iters[copy_index]);
	this->val_average_max_updates.push_back(this->val_average_max_updates[copy_index]);
	this->select_networks.push_back(new Network(this->select_networks[copy_index]));
	this->select_networks.back()->twiddle();
}

void PredictWrapper::save(ofstream& output_file) {
	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		this->val_networks[n_index]->save(output_file);
		output_file << this->val_average_max_updates[n_index] << endl;

		this->select_networks[n_index]->save(output_file);
	}

	output_file << this->misguess_average << endl;
}
