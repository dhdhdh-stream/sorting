#include "fold_helper.h"

#include <iostream>

#include "solution_node_action.h"
#include "solution_node_empty.h"

using namespace std;

FoldHelper::FoldHelper(SolutionNode* parent,
					   int layer) {
	this->parent = parent;
	this->input_indexes = new NDVector(layer);
	this->new_state_network = NULL;
}

FoldHelper::~FoldHelper() {
	delete this->input_indexes;
	if (this->new_state_network != NULL) {
		delete this->new_state_network;
	}
}

void FoldHelper::set_index(vector<int>& loop_scope_counts,
						   int& curr_index) {
	this->input_indexes->set_value(loop_scope_counts,
								   0,
								   curr_index);
}

void FoldHelper::initialize_new_state_network(int new_state_size) {
	this->new_state_size = new_state_size;
	this->new_state_network = new Network(1+this->new_state_size,
										  4*(1+this->new_state_size),
										  this->new_state_size);
}

int FoldHelper::get_index(vector<int>& loop_scope_counts) {
	int index;
	this->input_indexes->get_value(loop_scope_counts, 0, index);

	return index;
}

void FoldHelper::new_path_process(vector<int>& loop_scope_counts,
								  int input_index_on,
								  double observations,
								  vector<vector<double>>& state_vals,
								  vector<AbstractNetworkHistory*>& network_historys) {
	int index;
	this->input_indexes->get_value(loop_scope_counts, 0, index);
	if (index > input_index_on) {
		// do nothing
	} else {
		SolutionNodeEmpty* parent_empty = (SolutionNodeEmpty*)this->parent;
		parent_empty->new_path_activate_state_networks(observations,
													   state_vals,
													   network_historys);
	}
}

void FoldHelper::new_path_process(vector<int>& loop_scope_counts,
								  int input_index_on,
								  double observations,
								  double* flat_inputs,
								  bool* activated,
								  vector<vector<double>>& state_vals,
								  vector<AbstractNetworkHistory*>& network_historys) {
	int index;
	this->input_indexes->get_value(loop_scope_counts, 0, index);
	if (index > input_index_on) {
		flat_inputs[index] = observations;
		activated[index] = true;
	} else {
		SolutionNodeAction* parent_action = (SolutionNodeAction*)this->parent;
		parent_action->new_path_activate_state_networks(observations,
														state_vals,
														network_historys);

		flat_inputs[index] = 0.0;
		activated[index] = true;
	}
}

void FoldHelper::existing_path_process(std::vector<int>& loop_scope_counts,
									   int input_index_on,
									   double observations,
									   std::vector<double>& new_state_vals,
									   std::vector<AbstractNetworkHistory*>& network_historys) {
	int index;
	this->input_indexes->get_value(loop_scope_counts, 0, index);
	if (index > input_index_on) {
		// do nothing
	} else {
		vector<double> inputs;
		inputs.reserve(1+this->new_state_size);
		inputs.push_back(observations);
		for (int s_index = 0; s_index < this->new_state_size; s_index++) {
			inputs.push_back(new_state_vals[s_index]);
		}

		this->new_state_network->mtx.lock();
		this->new_state_network->activate(inputs, network_historys);
		for (int s_index = 0; s_index < this->new_state_size; s_index++) {
			new_state_vals[s_index] = this->new_state_network->output->acti_vals[s_index];
		}
		this->new_state_network->mtx.unlock();
	}
}

void FoldHelper::existing_path_process(std::vector<int>& loop_scope_counts,
									   int input_index_on,
									   double observations,
									   double* flat_inputs,
									   bool* activated,
									   std::vector<double>& new_state_vals,
									   std::vector<AbstractNetworkHistory*>& network_historys) {
	int index;
	this->input_indexes->get_value(loop_scope_counts, 0, index);
	if (index > input_index_on) {
		flat_inputs[index] = observations;
		activated[index] = true;
	} else {
		vector<double> inputs;
		inputs.reserve(1+this->new_state_size);
		inputs.push_back(observations);
		for (int s_index = 0; s_index < this->new_state_size; s_index++) {
			inputs.push_back(new_state_vals[s_index]);
		}

		this->new_state_network->mtx.lock();
		this->new_state_network->activate(inputs, network_historys);
		for (int s_index = 0; s_index < this->new_state_size; s_index++) {
			new_state_vals[s_index] = this->new_state_network->output->acti_vals[s_index];
		}
		this->new_state_network->mtx.unlock();

		flat_inputs[index] = 0.0;
		activated[index] = true;
	}
}

void FoldHelper::activate_new_state_network(double observations,
											vector<double>& new_state_vals) {
	vector<double> inputs;
	inputs.reserve(1+this->new_state_size);
	inputs.push_back(observations);
	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		inputs.push_back(new_state_vals[s_index]);
	}

	this->new_state_network->mtx.lock();
	this->new_state_network->activate(inputs);
	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		new_state_vals[s_index] = this->new_state_network->output->acti_vals[s_index];
	}
	this->new_state_network->mtx.unlock();
}

void FoldHelper::existing_path_backprop_new_state(std::vector<double>& new_state_errors,
												  std::vector<AbstractNetworkHistory*>& network_historys) {
	vector<double> errors;
	errors.reserve(this->new_state_size);
	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		errors.push_back(new_state_errors[s_index]);
	}

	AbstractNetworkHistory* network_history = network_historys.back();

	this->new_state_network->mtx.lock();

	network_history->reset_weights();

	this->new_state_network->backprop(errors);

	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		new_state_errors[s_index] = this->new_state_network->input->errors[1+s_index];
		this->new_state_network->input->errors[1+s_index] = 0.0;
	}

	this->new_state_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

NDVector::NDVector(int height) {
	this->height = height;
	if (this->height > 1) {
		for (int i = 0; i < 5; i++) {
			this->inner.push_back(new NDVector(height-1));
		}
	}
}

NDVector::~NDVector() {
	if (this->height > 1) {
		for (int i = 0; i < 5; i++) {
			delete this->inner[i];
		}
	}
}

void NDVector::set_value(vector<int>& index,
						 int curr,
						 int value) {
	if (this->height == 1) {
		this->value = value;
		return;
	}

	this->inner[index[curr]]->set_value(index, curr+1, value);
}

void NDVector::get_value(vector<int>& index,
						 int curr,
						 int& value) {
	if (this->height == 1) {
		value = this->value;
		return;
	}

	this->inner[index[curr]]->get_value(index, curr+1, value);
}
