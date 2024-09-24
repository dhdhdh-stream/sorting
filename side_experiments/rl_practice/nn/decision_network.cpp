#include "decision_network.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 20;

DecisionNetwork::DecisionNetwork(int num_obs,
								 int num_actions,
								 int num_state) {
	this->obs_input = new Layer(num_obs);
	for (int i_index = 0; i_index < num_obs; i_index++) {
		this->obs_input->acti_vals.push_back(0.0);
		this->obs_input->errors.push_back(0.0);
	}

	this->state_input = new Layer(num_state);
	for (int i_index = 0; i_index < num_state; i_index++) {
		this->state_input->acti_vals.push_back(0.0);
		this->state_input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 10; h_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->obs_input);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 4; h_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->obs_input);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->output = new SoftmaxLayer(num_actions);
	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->output->acti_vals.push_back(0.0);
	}
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();

	this->epoch_iter = 0;
	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

DecisionNetwork::DecisionNetwork(ifstream& input_file) {
	this->obs_input = new Layer(LINEAR_LAYER);
	string obs_input_size_line;
	getline(input_file, obs_input_size_line);
	int obs_input_size = stoi(obs_input_size_line);
	for (int i_index = 0; i_index < obs_input_size; i_index++) {
		this->obs_input->acti_vals.push_back(0.0);
		this->obs_input->errors.push_back(0.0);
	}

	this->state_input = new Layer(LINEAR_LAYER);
	string state_input_size_line;
	getline(input_file, state_input_size_line);
	int state_input_size = stoi(state_input_size_line);
	for (int i_index = 0; i_index < state_input_size; i_index++) {
		this->state_input->acti_vals.push_back(0.0);
		this->state_input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	for (int i_index = 0; i_index < hidden_1_size; i_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->obs_input);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	for (int i_index = 0; i_index < hidden_2_size; i_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->obs_input);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	string num_actions_line;
	getline(input_file, num_actions_line);
	int num_actions = stoi(num_actions_line);
	this->output = new SoftmaxLayer(num_actions);
	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->output->acti_vals.push_back(0.0);
	}
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->epoch_iter = 0;
	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

DecisionNetwork::~DecisionNetwork() {
	delete this->obs_input;
	delete this->state_input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void DecisionNetwork::activate(vector<double>& obs_vals,
							   vector<double>& state_vals,
							   int& action) {
	for (int i_index = 0; i_index < (int)obs_vals.size(); i_index++) {
		this->obs_input->acti_vals[i_index] = obs_vals[i_index];
	}
	for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
		this->state_input[s_index] = state_vals[s_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	action = this->output->predicted_index - 1;
}

void DecisionNetwork::backprop(vector<double>& obs_vals,
							   int action,
							   vector<double>& state_vals,
							   bool is_better,
							   vector<double>& state_errors) {
	for (int i_index = 0; i_index < (int)obs_vals.size(); i_index++) {
		this->obs_input->acti_vals[i_index] = obs_vals[i_index];
	}
	for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
		this->state_input[s_index] = state_vals[s_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate(action);

	this->output->backprop(is_better);
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	for (int i_index = 0; i_index < (int)this->state_input->acti_vals.size(); i_index++) {
		state_errors[i_index] += this->state_input->errors[i_index];
		this->state_input->errors[i_index] = 0.0;
	}

	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		double hidden_1_max_update = 0.0;
		this->hidden_1->get_max_update(hidden_1_max_update);
		this->hidden_1_average_max_update = 0.999*this->hidden_1_average_max_update+0.001*hidden_1_max_update;
		if (hidden_1_max_update > 0.0) {
			double hidden_1_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_1_average_max_update;
			if (hidden_1_learning_rate*hidden_1_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_1_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_1_max_update;
			}
			this->hidden_1->update_weights(hidden_1_learning_rate);
		}

		double hidden_2_max_update = 0.0;
		this->hidden_2->get_max_update(hidden_2_max_update);
		this->hidden_2_average_max_update = 0.999*this->hidden_2_average_max_update+0.001*hidden_2_max_update;
		if (hidden_2_max_update > 0.0) {
			double hidden_2_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_2_average_max_update;
			if (hidden_2_learning_rate*hidden_2_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_2_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_2_max_update;
			}
			this->hidden_2->update_weights(hidden_2_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > NETWORK_TARGET_MAX_UPDATE) {
				output_learning_rate = NETWORK_TARGET_MAX_UPDATE/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void DecisionNetwork::save(ofstream& output_file) {
	output_file << this->obs_input->acti_vals.size() << endl;
	output_file << this->state_input->acti_vals.size() << endl;
	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;
	output_file << this->output->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->output->save_weights(output_file);
}
