#include "fold_helper.h"

using namespace std;

FoldHelper::FoldHelper(SolutionNode* parent,
					   int layer) {
	this->parent = parent;
	this->layer = layer;
	this->input_indexes = new NDVector(layer);
	this->potential_state_network = NULL;
}

FoldHelper::~FoldHelper() {
	delete this->input_indexes;
	if (this->potential_state_network != NULL) {
		delete this->potential_state_network;
	}
}

void FoldHelper::set_index(vector<int>& loop_scope_counts,
						   int& curr_index) {
	this->input_indexes->set_value(loop_scope_counts,
								   0,
								   curr_index);
}

void FoldHelper::process(vector<int>& loop_scope_counts,
						 double observations,
						 int input_index_on,
						 double* flat_inputs,
						 bool* activated,
						 vector<double>& new_state_vals,
						 vector<SolutionNode*>& backprop_nodes,
						 vector<AbstractNetworkHistory*>& network_historys) {
	int index;
	this->input_indexes->get_value(loop_scope_counts, 0, index);
	if (index >= input_index_on) {
		network_input[index] = obs;
	} else {
		vector<double> network_inputs;
		network_inputs.push_back(observations);
		for (int s_index = 0; s_index < (int)new_state_vals.size(); s_index++) {
			network_inputs.push_back(new_state_vals[s_index]);
		}

		this->potential_state_network->mtx.lock();
		this->potential_state_network->activate(network_inputs, network_historys)
		for (int s_index = 0; s_index < (int)new_state_vals.size(); s_index++) {
			new_state_vals[s_index] = this->potential_state_network->output->acti_vals[s_index];
		}
		this->potential_state_network->mtx.unlock();

		backprop_nodes.push_back(this->parent);
	}
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
	for (int i = 0; i < 5; i++) {
		delete this->inner[i];
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
