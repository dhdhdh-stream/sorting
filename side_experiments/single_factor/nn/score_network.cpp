#include "score_network.h"

using namespace std;

void ScoreNetwork::construct() {
	this->state_input = new Layer(LINEAR_LAYER, (int)this->context_indexes.size());

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->state_input);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->output_average_max_update = 0.0;
}

ScoreNetwork::ScoreNetwork(vector<int> context_indexes,
						   vector<int> state_indexes) {
	this->context_indexes = context_indexes;
	this->state_indexes = state_indexes;

	construct();
}

ScoreNetwork::ScoreNetwork(ScoreNetwork* original) {
	this->context_indexes = original->context_indexes;
	this->state_indexes = original->state_indexes;

	construct();

	this->output->copy_weights_from(original->output);
}

ScoreNetwork::ScoreNetwork(ifstream& input_file) {
	string state_size_line;
	getline(input_file, state_size_line);
	int state_size = stoi(state_size_line);
	for (int s_index = 0; s_index < state_size; s_index++) {
		string context_index_line;
		getline(input_file, context_index_line);
		this->context_indexes.push_back(stoi(context_index_line));

		string state_index_line;
		getline(input_file, state_index_line);
		this->state_indexes.push_back(stoi(state_index_line));
	}

	construct();

	this->output->load_weights_from(input_file);
}

ScoreNetwork::~ScoreNetwork() {
	delete this->state_input;
	delete this->output;
}

void ScoreNetwork::activate(vector<vector<double>*>& state_vals) {
	for (int s_index = 0; s_index < (int)this->context_indexes.size(); s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[this->context_indexes[s_index]]->at(this->state_indexes[s_index]);
	}

	this->output->activate();
}

void ScoreNetwork::activate(vector<vector<double>*>& state_vals,
							ScoreNetworkHistory* history) {
	activate(state_vals);

	history->save_weights();
}

void ScoreNetwork::backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop_weights_with_no_error_signal();

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > target_max_update) {
				output_learning_rate = target_max_update/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void ScoreNetwork::backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update,
		ScoreNetworkHistory* history) {
	history->reset_weights();

	backprop_weights_with_no_error_signal(output_error,
										  target_max_update);
}

void ScoreNetwork::backprop_errors_with_no_weight_change(
		double output_error,
		vector<vector<double>*>& state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop_errors_with_no_weight_change();

	for (int s_index = 0; s_index < (int)this->context_indexes.size(); s_index++) {
		state_errors[this->context_indexes[s_index]]->at(this->state_indexes[s_index]) += this->state_input->errors[s_index];
	}
}

void ScoreNetwork::backprop_errors_with_no_weight_change(
		double output_error,
		vector<vector<double>*>& state_errors,
		ScoreNetworkHistory* history) {
	history->reset_weights();

	backprop_errors_with_no_weight_change(output_error,
										  state_errors);
}

void ScoreNetwork::add_state(int context_index,
							 int state_index) {
	this->context_indexes.push_back(context_index);
	this->state_indexes.push_back(state_index);
	this->state_input->acti_vals.push_back(0.0);
	this->state_input->errors.push_back(0.0);

	this->output->weights[0][0].push_back(0.0);
	this->output->weight_updates[0][0].push_back(0.0);
}

void ScoreNetwork::save(ofstream& output_file) {
	output_file << this->context_indexes.size() << endl;
	for (int s_index = 0; s_index < (int)this->context_indexes.size(); s_index++) {
		output_file << this->context_indexes[s_index] << endl;
		output_file << this->state_indexes[s_index] << endl;
	}

	this->output->save_weights(output_file);
}

ScoreNetworkHistory::ScoreNetworkHistory(ScoreNetwork* network) {
	this->network = network;
}

void ScoreNetworkHistory::save_weights() {
	this->state_input_history.reserve(this->network->context_indexes.size());
	for (int s_index = 0; s_index < (int)this->network->context_indexes.size(); s_index++) {
		this->state_input_history.push_back(this->network->state_input->acti_vals[s_index]);
	}
}

void ScoreNetworkHistory::reset_weights() {
	for (int s_index = 0; s_index < (int)this->network->context_indexes.size(); s_index++) {
		this->network->state_input->acti_vals[s_index] = this->state_input_history[s_index];
	}
}
