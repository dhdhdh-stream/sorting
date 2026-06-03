#include "world_model.h"

#include "network.h"

using namespace std;

const int STARTING_NUM_STATE = 2;

const int NUM_STATE_CHANGE = 2;

WorldModel::WorldModel(int num_obs,
					   int num_actions) {
	this->num_obs = num_obs;
	this->num_actions = num_actions;

	this->num_states = STARTING_NUM_STATE;

	this->obs_network = new Network(STARTING_NUM_STATE + this->num_obs, STARTING_NUM_STATE);
	this->action_network = new Network(STARTING_NUM_STATE + this->num_actions, STARTING_NUM_STATE);

	this->score_network = new Network(STARTING_NUM_STATE, 1);

	this->misguess_average = 0.0;
	this->misguess_variance_average = 0.0;

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

WorldModel::WorldModel(WorldModel* original) {
	this->num_obs = original->num_obs;
	this->num_actions = original->num_actions;

	this->num_states = original->num_states;

	this->obs_network = new Network(original->obs_network);
	this->action_network = new Network(original->action_network);

	this->score_network = new Network(original->score_network);

	this->misguess_average = original->misguess_average;
	this->misguess_variance_average = original->misguess_variance_average;

	this->epoch_iter = 0;
	this->average_max_update = original->average_max_update;
}

WorldModel::~WorldModel() {
	delete this->obs_network;
	delete this->action_network;

	delete this->score_network;
}

void WorldModel::add_states() {
	this->num_states += NUM_STATE_CHANGE;

	this->obs_network->add_inputs(NUM_STATE_CHANGE);
	this->obs_network->add_outputs(NUM_STATE_CHANGE);
	this->action_network->add_inputs(NUM_STATE_CHANGE);
	this->action_network->add_outputs(NUM_STATE_CHANGE);

	this->score_network->add_inputs(NUM_STATE_CHANGE);
}

void WorldModel::remove_states() {
	this->num_states -= NUM_STATE_CHANGE;

	this->obs_network->remove_inputs(NUM_STATE_CHANGE);
	this->obs_network->remove_outputs(NUM_STATE_CHANGE);
	this->action_network->remove_inputs(NUM_STATE_CHANGE);
	this->action_network->remove_outputs(NUM_STATE_CHANGE);

	this->score_network->remove_inputs(NUM_STATE_CHANGE);
}
