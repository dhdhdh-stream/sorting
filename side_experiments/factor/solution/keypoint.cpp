#include "keypoint.h"

#include <cmath>

#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

Keypoint::Keypoint() {
	// do nothing
}

Keypoint::Keypoint(Keypoint* original,
				   Solution* parent_solution) {
	this->inputs = original->inputs;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			this->inputs[i_index].scope_context[l_index] =
				parent_solution->scopes[this->inputs[i_index].scope_context[l_index]->id];
		}
	}
	this->network = new Network(original->network);

	this->misguess_standard_deviation = original->misguess_standard_deviation;

	this->availability = original->availability;
}

Keypoint::~Keypoint() {
	delete network;
}

bool Keypoint::should_clean_inputs(Scope* scope,
								   int node_id) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == scope
					&& this->inputs[i_index].node_context[l_index] == node_id) {
				return true;
			}
		}
	}

	return false;
}

bool Keypoint::should_clean_inputs(Scope* scope) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == scope) {
				return true;
			}
		}
	}

	return false;
}

void Keypoint::clean() {
	this->num_hit = 0;
	this->num_miss = 0;
	this->sum_misguess = 0.0;
}

void Keypoint::measure_update() {
	this->misguess_standard_deviation = sqrt(this->sum_misguess / this->num_hit);

	this->availability = (double)this->num_hit / ((double)this->num_hit + (double)this->num_miss);
}

void Keypoint::save(ofstream& output_file) {
	output_file << this->inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		this->inputs[i_index].save(output_file);
	}

	this->network->save(output_file);

	output_file << this->misguess_standard_deviation << endl;

	output_file << this->availability << endl;
}

void Keypoint::load(ifstream& input_file,
					Solution* parent_solution) {
	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		this->inputs.push_back(Input(input_file,
									 parent_solution));
	}

	this->network = new Network(input_file);

	string misguess_standard_deviation_line;
	getline(input_file, misguess_standard_deviation_line);
	this->misguess_standard_deviation = stod(misguess_standard_deviation_line);

	string availability_line;
	getline(input_file, availability_line);
	this->availability = stod(availability_line);
}
