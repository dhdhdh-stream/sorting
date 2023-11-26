#include "full_network.h"

#include "forget_network.h"
#include "index_network.h"
#include "state.h"
#include "state_network.h"
#include "state_status.h"
#include "update_size_network.h"

using namespace std;

FullNetwork::FullNetwork(int index) {
	this->update_size_network = new UpdateSizeNetwork();
	this->state_network = new StateNetwork();
	this->forget_network = new ForgetNetwork();
	this->index_network = new IndexNetwork();

	this->index = index;

	this->starting_mean = 0.0;
	this->starting_variance = 1.0;

	this->ending_mean = 0.0;
	this->ending_variance = 1.0;

	this->average_update_size = 1.0;
	this->average_index = 0.0;
}

FullNetwork::FullNetwork(ifstream& input_file,
						 State* parent_state,
						 int index) {
	ifstream update_size_network_save_file;
	update_size_network_save_file.open("saves/nns/" + to_string(parent_state->id) + "_" + to_string(index) + "_update_size.txt");
	this->update_size_network = new UpdateSizeNetwork(update_size_network_save_file);
	update_size_network_save_file.close();

	ifstream state_network_save_file;
	state_network_save_file.open("saves/nns/" + to_string(parent_state->id) + "_" + to_string(index) + "_state.txt");
	this->state_network = new StateNetwork(state_network_save_file);
	state_network_save_file.close();

	ifstream forget_network_save_file;
	forget_network_save_file.open("saves/nns/" + to_string(parent_state->id) + "_" + to_string(index) + "_forget.txt");
	this->forget_network = new ForgetNetwork(forget_network_save_file);
	forget_network_save_file.close();

	ifstream index_network_save_file;
	index_network_save_file.open("saves/nns/" + to_string(parent_state->id) + "_" + to_string(index) + "_index.txt");
	this->index_network = new IndexNetwork(index_network_save_file);
	index_network_save_file.close();

	this->parent_state = parent_state;
	this->index = index;

	string preceding_network_indexes_size_line;
	getline(input_file, preceding_network_indexes_size_line);
	int preceding_network_indexes_size = stoi(preceding_network_indexes_size_line);
	for (int n_index = 0; n_index < preceding_network_indexes_size; n_index++) {
		string network_index_line;
		getline(input_file, network_index_line);
		this->preceding_network_indexes.insert(stoi(network_index_line));
	}

	string starting_mean_line;
	getline(input_file, starting_mean_line);
	this->starting_mean = stod(starting_mean_line);

	string starting_standard_deviation_line;
	getline(input_file, starting_standard_deviation_line);
	this->starting_standard_deviation = stod(starting_standard_deviation_line);

	string ending_mean_line;
	getline(input_file, ending_mean_line);
	this->ending_mean = stod(ending_mean_line);

	string ending_standard_deviation_line;
	getline(input_file, ending_standard_deviation_line);
	this->ending_standard_deviation = stod(ending_standard_deviation_line);

	string average_index_line;
	getline(input_file, average_index_line);
	this->average_index = stod(average_index_line);
}

FullNetwork::~FullNetwork() {
	delete this->update_size_network;
	delete this->state_network;
	delete this->forget_network;
	delete this->index_network;
}

void FullNetwork::activate(double obs_val,
						   double& state_val,
						   double& index_val,
						   FullNetworkHistory* history) {
	history->starting_index_val_snapshot = index_val;

	history->update_size_network_history = new UpdateSizeNetworkHistory(this->update_size_network);
	this->update_size_network->activate(obs_val,
										index_val,
										history->update_size_network_history);
	history->update_size_snapshot = this->update_size_network->output->acti_vals[0];

	history->state_network_history = new StateNetworkHistory(this->state_network);
	this->state_network->activate(obs_val,
								  state_val,
								  history->state_network_history);
	history->state_update_snapshot = this->state_network->output->acti_vals[0];

	state_val += this->update_size_network->output->acti_vals[0] * this->state_network->output->acti_vals[0];

	history->forget_network_history = new ForgetNetworkHistory(this->forget_network);
	this->forget_network->activate(obs_val,
								   index_val,
								   history->forget_network_history);

	index_val += this->forget_network->output->acti_vals[0] * (-index_val);

	history->index_network_history = new IndexNetworkHistory(this->index_network);
	this->index_network->activate(obs_val,
								  history->index_network_history);

	index_val += this->index_network->output->acti_vals[0];

	index_val -= 0.1;

	this->average_update_size = 0.999*this->average_update_size + 0.001*history->update_size_snapshot;
	double curr_weight = history->update_size_snapshot / this->average_update_size;
	this->average_index = 0.999*this->average_index
		+ 0.001*curr_weight*history->starting_index_val_snapshot;
}

void FullNetwork::backprop(double& state_error,
						   double& index_error,
						   FullNetworkHistory* history) {
	this->index_network->backprop(index_error,
								  history->index_network_history);

	this->forget_network->backprop((-history->starting_index_val_snapshot)*index_error,
								   history->forget_network_history);

	this->state_network->backprop(history->update_size_snapshot*state_error,
								  history->state_network_history);
	this->update_size_network->backprop(history->state_update_snapshot*state_error,
										history->update_size_network_history);

	index_error += this->forget_network->index_input->errors[0];
	this->forget_network->index_input->errors[0] = 0.0;

	state_error += this->state_network->state_input->errors[0];
	this->state_network->state_input->errors[0] = 0.0;

	index_error += this->update_size_network->index_input->errors[0];
	this->update_size_network->index_input->errors[0] = 0.0;
}

void FullNetwork::activate(double obs_val,
						   double& state_val,
						   double& index_val) {
	this->update_size_network->activate(obs_val,
										index_val);

	this->state_network->activate(obs_val,
								  state_val);

	state_val += this->update_size_network->output->acti_vals[0] * this->state_network->output->acti_vals[0];

	this->forget_network->activate(obs_val,
								   index_val);

	index_val += this->forget_network->output->acti_vals[0] * (-index_val);

	this->index_network->activate(obs_val);

	index_val += this->index_network->output->acti_vals[0];

	index_val -= 0.1;
}

void FullNetwork::activate(double obs_val,
						   StateStatus& state_status) {
	double starting_state_val;
	double starting_index_val;
	FullNetwork* last_network = state_status.last_network;
	if (last_network == NULL) {
		starting_state_val = (state_status.val * this->starting_standard_deviation) + this->starting_mean;
		starting_index_val = state_status.index + this->average_index;
	} else if (this->parent_state != last_network->parent_state
			|| this->preceding_network_indexes.find(last_network->index) == this->preceding_network_indexes.end()) {
		double normalized = (state_status.val - last_network->ending_mean)
			/ last_network->ending_standard_deviation;
		starting_state_val = (normalized * this->starting_standard_deviation) + this->starting_mean;

		starting_index_val = state_status.index - last_network->average_index + this->average_index;
	} else {
		starting_state_val = state_status.val;
		starting_index_val = state_status.index;
	}

	this->update_size_network->activate(obs_val,
										starting_index_val);

	this->state_network->activate(obs_val,
								  starting_state_val);

	state_status.val = starting_state_val + this->update_size_network->output->acti_vals[0] * this->state_network->output->acti_vals[0];

	this->forget_network->activate(obs_val,
								   starting_index_val);

	this->index_network->activate(obs_val);

	state_status.index = starting_index_val
		+ this->forget_network->output->acti_vals[0] * (-starting_index_val)
		+ this->index_network->output->acti_vals[0]
		- 0.1;
}

void FullNetwork::save(std::ofstream& output_file) {
	ofstream update_size_network_save_file;
	update_size_network_save_file.open("saves/nns/" + to_string(parent_state->id) + "_" + to_string(index) + "_update_size.txt");
	this->update_size_network->save(update_size_network_save_file);
	update_size_network_save_file.close();

	ofstream state_network_save_file;
	state_network_save_file.open("saves/nns/" + to_string(parent_state->id) + "_" + to_string(index) + "_state.txt");
	this->state_network->save(state_network_save_file);
	state_network_save_file.close();

	ofstream forget_network_save_file;
	forget_network_save_file.open("saves/nns/" + to_string(parent_state->id) + "_" + to_string(index) + "_forget.txt");
	this->forget_network->save(forget_network_save_file);
	forget_network_save_file.close();

	ofstream index_network_save_file;
	index_network_save_file.open("saves/nns/" + to_string(parent_state->id) + "_" + to_string(index) + "_index.txt");
	this->index_network->save(index_network_save_file);
	index_network_save_file.close();

	output_file << this->preceding_network_indexes.size() << endl;
	for (set<int>::iterator it = preceding_network_indexes.begin();
			it != preceding_network_indexes.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->starting_mean << endl;
	output_file << this->starting_standard_deviation << endl;
	output_file << this->ending_mean << endl;
	output_file << this->ending_standard_deviation << endl;
	output_file << this->average_index << endl;
}

FullNetworkHistory::FullNetworkHistory(FullNetwork* network) {
	this->network = network;
}

FullNetworkHistory::~FullNetworkHistory() {
	delete this->update_size_network_history;
	delete this->state_network_history;
	delete this->forget_network_history;
	delete this->index_network_history;
}
