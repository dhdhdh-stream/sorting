#include "gate.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;

Gate::Gate(int num_obs,
		   int num_actions,
		   int num_states) {
	this->obs_input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < num_obs; i_index++) {
		this->obs_input->acti_vals.push_back(0.0);
		this->obs_input->errors.push_back(0.0);
	}

	this->action_input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < num_actions; i_index++) {
		this->action_input->acti_vals.push_back(0.0);
		this->action_input->errors.push_back(0.0);
	}

	this->state_input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < num_states; i_index++) {
		this->state_input->acti_vals.push_back(0.0);
		this->state_input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 4; h_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->obs_input);
	this->hidden_1->input_layers.push_back(this->action_input);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 1; h_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->obs_input);
	this->hidden_2->input_layers.push_back(this->action_input);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->output = new Layer(SIGMOID_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->obs_input);
	this->output->input_layers.push_back(this->action_input);
	this->output->input_layers.push_back(this->state_input);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();

	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Gate::Gate(ifstream& input_file) {
	this->obs_input = new Layer(LINEAR_LAYER);
	string obs_input_size_line;
	getline(input_file, obs_input_size_line);
	int obs_input_size = stoi(obs_input_size_line);
	for (int i_index = 0; i_index < obs_input_size; i_index++) {
		this->obs_input->acti_vals.push_back(0.0);
		this->obs_input->errors.push_back(0.0);
	}

	this->action_input = new Layer(LINEAR_LAYER);
	string action_input_size_line;
	getline(input_file, action_input_size_line);
	int action_input_size = stoi(action_input_size_line);
	for (int i_index = 0; i_index < action_input_size; i_index++) {
		this->action_input->acti_vals.push_back(0.0);
		this->action_input->errors.push_back(0.0);
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
	this->hidden_1->input_layers.push_back(this->action_input);
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
	this->hidden_2->input_layers.push_back(this->action_input);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->output = new Layer(SIGMOID_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->obs_input);
	this->output->input_layers.push_back(this->action_input);
	this->output->input_layers.push_back(this->state_input);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Gate::~Gate() {
	delete this->obs_input;
	delete this->action_input;
	delete this->state_input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void Gate::activate(vector<double>& obs_vals,
					int action,
					std::vector<double>& state_vals) {
	for (int i_index = 0; i_index < (int)obs_vals.size(); i_index++) {
		this->obs_input->acti_vals[i_index] = obs_vals[i_index];
	}
	for (int i_index = 0; i_index < (int)this->action_input->acti_vals.size(); i_index++) {
		if (i_index == action) {
			this->action_input->acti_vals[i_index] = 1.0;
		} else {
			this->action_input->acti_vals[i_index] = 0.0;
		}
	}
	for (int i_index = 0; i_index < (int)state_vals.size(); i_index++) {
		this->state_input->acti_vals[i_index] = state_vals[i_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();
}

void Gate::activate(vector<double>& obs_vals,
					int action,
					std::vector<double>& state_vals,
					GateHistory* history) {
	for (int i_index = 0; i_index < (int)obs_vals.size(); i_index++) {
		this->obs_input->acti_vals[i_index] = obs_vals[i_index];
	}
	for (int i_index = 0; i_index < (int)this->action_input->acti_vals.size(); i_index++) {
		if (i_index == action) {
			this->action_input->acti_vals[i_index] = 1.0;
		} else {
			this->action_input->acti_vals[i_index] = 0.0;
		}
	}
	for (int i_index = 0; i_index < (int)state_vals.size(); i_index++) {
		this->state_input->acti_vals[i_index] = state_vals[i_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	history->obs_input_histories = vector<double>(this->obs_input->acti_vals.size());
	for (int i_index = 0; i_index < (int)this->obs_input->acti_vals.size(); i_index++) {
		history->obs_input_histories[i_index] = this->obs_input->acti_vals[i_index];
	}
	history->action_input_histories = vector<double>(this->action_input->acti_vals.size());
	for (int i_index = 0; i_index < (int)this->action_input->acti_vals.size(); i_index++) {
		history->action_input_histories[i_index] = this->action_input->acti_vals[i_index];
	}
	history->state_input_histories = vector<double>(this->state_input->acti_vals.size());
	for (int i_index = 0; i_index < (int)this->state_input->acti_vals.size(); i_index++) {
		history->state_input_histories[i_index] = this->state_input->acti_vals[i_index];
	}
	history->hidden_1_histories = vector<double>(this->hidden_1->acti_vals.size());
	for (int n_index = 0; n_index < (int)this->hidden_1->acti_vals.size(); n_index++) {
		history->hidden_1_histories[n_index] = this->hidden_1->acti_vals[n_index];
	}
	history->hidden_2_histories = vector<double>(this->hidden_2->acti_vals.size());
	for (int n_index = 0; n_index < (int)this->hidden_2->acti_vals.size(); n_index++) {
		history->hidden_2_histories[n_index] = this->hidden_2->acti_vals[n_index];
	}
	history->output_histories = vector<double>(this->output->acti_vals.size());
	for (int n_index = 0; n_index < (int)this->output->acti_vals.size(); n_index++) {
		history->output_histories[n_index] = this->output->acti_vals[n_index];
	}
}

void Gate::backprop(double error,
					vector<double>& state_errors,
					GateHistory* history) {
	for (int i_index = 0; i_index < (int)this->obs_input->acti_vals.size(); i_index++) {
		this->obs_input->acti_vals[i_index] = history->obs_input_histories[i_index];
	}
	for (int i_index = 0; i_index < (int)this->action_input->acti_vals.size(); i_index++) {
		this->action_input->acti_vals[i_index] = history->action_input_histories[i_index];
	}
	for (int i_index = 0; i_index < (int)this->state_input->acti_vals.size(); i_index++) {
		this->state_input->acti_vals[i_index] = history->state_input_histories[i_index];
	}
	for (int n_index = 0; n_index < (int)this->hidden_1->acti_vals.size(); n_index++) {
		this->hidden_1->acti_vals[n_index] = history->hidden_1_histories[n_index];
	}
	for (int n_index = 0; n_index < (int)this->hidden_2->acti_vals.size(); n_index++) {
		this->hidden_2->acti_vals[n_index] = history->hidden_2_histories[n_index];
	}
	for (int n_index = 0; n_index < (int)this->output->acti_vals.size(); n_index++) {
		this->output->acti_vals[n_index] = history->output_histories[n_index];
	}

	this->output->errors[0] = error;
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	for (int i_index = 0; i_index < (int)state_errors.size(); i_index++) {
		state_errors[i_index] += this->state_input->errors[i_index];
		this->state_input->errors[i_index] = 0;
	}
}

void Gate::update_weights() {
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
}

void Gate::save(ofstream& output_file) {
	output_file << this->obs_input->acti_vals.size() << endl;
	output_file << this->action_input->acti_vals.size() << endl;
	output_file << this->state_input->acti_vals.size() << endl;
	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->output->save_weights(output_file);
}
