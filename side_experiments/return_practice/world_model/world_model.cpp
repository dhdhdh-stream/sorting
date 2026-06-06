#include "world_model.h"

#include "constants.h"
#include "state_network.h"

using namespace std;

WorldModel::WorldModel(int num_obs,
					   int num_actions) {
	this->num_states = STARTING_NUM_STATE;

	this->obs_network = new StateNetwork(STARTING_NUM_STATE + num_obs, STARTING_NUM_STATE);
	this->action_network = new StateNetwork(STARTING_NUM_STATE + num_actions, STARTING_NUM_STATE);

	this->final_network = new StateNetwork(STARTING_NUM_STATE, 1);

	this->epoch_iter = 0;
	this->average_max_update = 0.0;

	this->misguess_average = 0.0;
	this->misguess_variance_average = 0.0;

	this->predict_network = new StateNetwork(STARTING_NUM_STATE + num_actions, STARTING_NUM_STATE);

	this->predict_epoch_iter = 0;
	this->predict_average_max_update = 0.0;
}

WorldModel::WorldModel(WorldModel* original) {
	this->num_states = original->num_states;

	this->obs_network = new StateNetwork(original->obs_network);
	this->action_network = new StateNetwork(original->action_network);

	this->final_network = new StateNetwork(original->final_network);

	this->epoch_iter = 0;
	this->average_max_update = original->average_max_update;

	this->misguess_average = original->misguess_average;
	this->misguess_variance_average = original->misguess_variance_average;

	this->predict_network = new StateNetwork(original->predict_network);

	this->predict_epoch_iter = 0;
	this->predict_average_max_update = original->predict_average_max_update;
}

WorldModel::WorldModel(ifstream& input_file) {
	string num_state_line;
	getline(input_file, num_state_line);
	this->num_states = stoi(num_state_line);

	this->obs_network = new StateNetwork(input_file);
	this->action_network = new StateNetwork(input_file);

	this->final_network = new StateNetwork(input_file);

	this->epoch_iter = 0;

	string average_max_update_line;
	getline(input_file, average_max_update_line);
	this->average_max_update = stod(average_max_update_line);

	string misguess_average_line;
	getline(input_file, misguess_average_line);
	this->misguess_average = stod(misguess_average_line);

	string misguess_variance_average_line;
	getline(input_file, misguess_variance_average_line);
	this->misguess_variance_average = stod(misguess_variance_average_line);

	this->predict_network = new StateNetwork(input_file);

	this->predict_epoch_iter = 0;

	string predict_average_max_update_line;
	getline(input_file, predict_average_max_update_line);
	this->predict_average_max_update = stod(predict_average_max_update_line);
}

WorldModel::~WorldModel() {
	delete this->obs_network;
	delete this->action_network;

	delete this->final_network;

	delete this->predict_network;
}

void WorldModel::add_states() {
	this->num_states += NUM_STATE_CHANGE;

	this->obs_network->add_inputs(NUM_STATE_CHANGE);
	this->obs_network->add_outputs(NUM_STATE_CHANGE);
	this->action_network->add_inputs(NUM_STATE_CHANGE);
	this->action_network->add_outputs(NUM_STATE_CHANGE);

	this->final_network->add_inputs(NUM_STATE_CHANGE);

	this->predict_network->add_inputs(NUM_STATE_CHANGE);
	this->predict_network->add_outputs(NUM_STATE_CHANGE);
}

void WorldModel::save(ofstream& output_file) {
	output_file << this->num_states << endl;

	this->obs_network->save(output_file);
	this->action_network->save(output_file);

	this->final_network->save(output_file);

	output_file << this->average_max_update << endl;

	output_file << this->misguess_average << endl;
	output_file << this->misguess_variance_average << endl;

	this->predict_network->save(output_file);

	output_file << this->predict_average_max_update << endl;
}
