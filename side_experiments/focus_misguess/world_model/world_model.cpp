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

	this->score_network_inputs = original->score_network_inputs;
	for (int n_index = 0; n_index < (int)original->score_networks.size(); n_index++) {
		this->score_networks.push_back(new Network(original->score_networks[n_index]));
	}
	this->misguess_network_inputs = original->misguess_network_inputs;
	for (int n_index = 0; n_index < (int)original->misguess_networks.size(); n_index++) {
		this->misguess_networks.push_back(new Network(original->misguess_networks[n_index]));
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

	for (int n_index = 0; n_index < (int)this->score_networks.size(); n_index++) {
		delete this->score_networks[n_index];
	}

	for (int n_index = 0; n_index < (int)this->misguess_networks.size(); n_index++) {
		delete this->misguess_networks[n_index];
	}
}
