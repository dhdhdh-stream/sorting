#include "transition_network.h"

#include "constants.h"

using namespace std;

TransitionNetwork::TransitionNetwork(int front_num_states,
									 int back_num_states) {
	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(front_num_states);
	this->state_input->errors.resize(front_num_states);
	this->state_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(16);
	this->hidden_1->errors.resize(16);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(8);
	this->hidden_2->errors.resize(8);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(back_num_states);
	this->output->errors.resize(back_num_states);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(0.0);

	this->num_instances = 0;
	this->last_update_iter = -1;
	this->epoch_iter = 0;
}

TransitionNetwork::TransitionNetwork(TransitionNetwork* original) {
	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(original->state_input->acti_vals.size());
	this->state_input->errors.resize(original->state_input->errors.size());
	this->state_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(original->hidden_1->acti_vals.size());
	this->hidden_1->errors.resize(original->hidden_1->errors.size());
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(original->hidden_2->acti_vals.size());
	this->hidden_2->errors.resize(original->hidden_2->errors.size());
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);
	this->hidden_2->copy_weights_from(original->hidden_2);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(original->output->acti_vals.size());
	this->output->errors.resize(original->output->errors.size());
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(0.0);
	this->output->copy_weights_from(original->output);

	this->num_instances = 0;
	this->last_update_iter = -1;
	this->epoch_iter = 0;
}

TransitionNetwork::TransitionNetwork(ifstream& input_file) {
	this->state_input = new Layer(LINEAR_LAYER);
	string front_num_states_line;
	getline(input_file, front_num_states_line);
	int front_num_states = stoi(front_num_states_line);
	this->state_input->acti_vals.resize(front_num_states);
	this->state_input->errors.resize(front_num_states);
	this->state_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	this->hidden_1->acti_vals.resize(hidden_1_size);
	this->hidden_1->errors.resize(hidden_1_size);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	this->hidden_2->acti_vals.resize(hidden_2_size);
	this->hidden_2->errors.resize(hidden_2_size);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);

	this->output = new Layer(LINEAR_LAYER);
	string back_num_states_line;
	getline(input_file, back_num_states_line);
	int back_num_states = stoi(back_num_states_line);
	this->output->acti_vals.resize(back_num_states);
	this->output->errors.resize(back_num_states);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(0.0);

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->num_instances = 0;
	this->last_update_iter = -1;
	this->epoch_iter = 0;
}

TransitionNetwork::~TransitionNetwork() {
	delete this->state_input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void TransitionNetwork::activate(Eigen::VectorXf& front_state_vals,
								 Eigen::VectorXf& back_state_vals) {
	this->state_input->acti_vals = front_state_vals;

	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	back_state_vals += this->output->acti_vals;
}

void TransitionNetwork::save(TransitionNetworkHistory* history) {
	history->state_input_history = this->state_input->acti_vals;
	history->hidden_1_history = this->hidden_1->acti_vals;
	history->hidden_2_history = this->hidden_2->acti_vals;
}

void TransitionNetwork::load(TransitionNetworkHistory* history) {
	this->state_input->acti_vals = history->state_input_history;
	this->hidden_1->acti_vals = history->hidden_1_history;
	this->hidden_2->acti_vals = history->hidden_2_history;
}

void TransitionNetwork::backprop(Eigen::VectorXf& back_state_errors,
								 Eigen::VectorXf& front_state_errors) {
	this->output->errors = back_state_errors;

	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	front_state_errors += this->state_input->errors;
	this->state_input->errors.setConstant(0.0);

	this->num_instances++;
}

void TransitionNetwork::update() {
	this->epoch_iter++;
	if (this->epoch_iter == UPDATE_EPOCH_SIZE) {
		this->hidden_1->update(this->num_instances,
							   STATE_LEARNING_RATE);
		this->hidden_2->update(this->num_instances,
							   STATE_LEARNING_RATE);
		this->output->update(this->num_instances,
							 STATE_LEARNING_RATE);

		this->num_instances = 0;
		this->epoch_iter = 0;
	}
}

void TransitionNetwork::clear_momentum() {
	this->hidden_1->clear_momentum();
	this->hidden_2->clear_momentum();
	this->output->clear_momentum();

	this->num_instances = 0;
	this->epoch_iter = 0;
}

void TransitionNetwork::add_front_states(int new_num_states) {
	this->state_input->acti_vals.resize(new_num_states);
	this->state_input->errors.resize(new_num_states);
	this->state_input->errors.setConstant(0.0);

	this->hidden_1->update_structure(0.0);
	this->hidden_2->update_structure(0.0);
}

void TransitionNetwork::add_back_states(int new_num_states) {
	this->output->acti_vals.resize(new_num_states);
	this->output->errors.resize(new_num_states);
	this->output->errors.setConstant(0.0);

	this->output->update_structure(0.0);
}

void TransitionNetwork::save(ofstream& output_file) {
	output_file << this->state_input->acti_vals.size() << endl;
	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;
	output_file << this->output->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->output->save_weights(output_file);
}
