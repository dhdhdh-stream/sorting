#include "world_model.h"

#include "constants.h"
#include "network.h"
#include "predict_wrapper.h"
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

	this->curr_predict = new PredictWrapper();

	this->candidate_predict = new PredictWrapper();
	this->candidate_iter = 0;
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

	this->curr_predict = new PredictWrapper(original->curr_predict);

	this->candidate_predict = new PredictWrapper(original->candidate_predict);
	this->candidate_iter = original->candidate_iter;
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

	this->curr_predict = new PredictWrapper(input_file);

	this->candidate_predict = new PredictWrapper(input_file);

	string candidate_iter_line;
	getline(input_file, candidate_iter_line);
	this->candidate_iter = stoi(candidate_iter_line);
}

WorldModel::~WorldModel() {
	delete this->obs_network;
	delete this->action_network;

	delete this->final_network;

	delete this->curr_predict;

	delete this->candidate_predict;
}

void WorldModel::add_states() {
	this->num_states += NUM_STATE_CHANGE;

	this->obs_network->add_inputs(NUM_STATE_CHANGE);
	this->obs_network->add_outputs(NUM_STATE_CHANGE);
	this->action_network->add_inputs(NUM_STATE_CHANGE);
	this->action_network->add_outputs(NUM_STATE_CHANGE);

	this->final_network->add_inputs(NUM_STATE_CHANGE);

	this->curr_predict->add_states();

	this->candidate_predict->add_states();
}

void WorldModel::save(ofstream& output_file) {
	output_file << this->num_states << endl;

	this->obs_network->save(output_file);
	this->action_network->save(output_file);

	this->final_network->save(output_file);

	output_file << this->average_max_update << endl;

	output_file << this->misguess_average << endl;
	output_file << this->misguess_variance_average << endl;

	this->curr_predict->save(output_file);

	this->candidate_predict->save(output_file);
	output_file << this->candidate_iter << endl;
}
