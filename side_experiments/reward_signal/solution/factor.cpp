#include "factor.h"

#include <cmath>
#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"

using namespace std;

Factor::Factor() {
	this->pcc = 0.0;
	this->num_success = 0;
	this->num_failure = 0;
	this->num_selected = 0;

	this->average = 0.0;
	this->standard_deviation = 0.0;
}

Factor::Factor(Factor* original,
			   Solution* parent_solution) {
	this->inputs = original->inputs;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			this->inputs[i_index].scope_context[l_index] =
				parent_solution->scopes[this->inputs[i_index].scope_context[l_index]->id];
		}
	}
	this->network = new Network(original->network);

	this->pcc = original->pcc;
	this->num_success = original->num_success;
	this->num_failure = original->num_failure;
	this->num_selected = original->num_selected;

	this->average = original->average;
	this->standard_deviation = original->standard_deviation;
}

Factor::~Factor() {
	delete this->network;
}

void Factor::clean_inputs(Scope* scope,
						  int node_id) {
	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == scope
					&& this->inputs[i_index].node_context[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->inputs.erase(this->inputs.begin() + i_index);
			this->network->remove_input(i_index);
		}
	}
}

void Factor::clean_inputs(Scope* scope) {
	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->inputs.erase(this->inputs.begin() + i_index);
			this->network->remove_input(i_index);
		}
	}
}

void Factor::replace_factor(Scope* scope,
							int original_node_id,
							int original_factor_index,
							int new_node_id,
							int new_factor_index) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		if (this->inputs[i_index].scope_context.back() == scope
				&& this->inputs[i_index].node_context.back() == original_node_id
				&& this->inputs[i_index].factor_index == original_factor_index) {
			this->inputs[i_index].node_context.back() = new_node_id;
			this->inputs[i_index].factor_index = new_factor_index;
		}
	}
}

void Factor::replace_obs_node(Scope* scope,
							  int original_node_id,
							  int new_node_id) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		if (this->inputs[i_index].scope_context.back() == scope
				&& this->inputs[i_index].node_context.back() == original_node_id) {
			this->inputs[i_index].node_context.back() = new_node_id;
		}
	}
}

void Factor::replace_scope(Scope* original_scope,
						   Scope* new_scope,
						   int new_scope_node_id) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		for (int l_index = 1; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == original_scope) {
				this->inputs[i_index].scope_context.insert(
					this->inputs[i_index].scope_context.begin() + l_index, new_scope);
				this->inputs[i_index].node_context.insert(
					this->inputs[i_index].node_context.begin() + l_index, new_scope_node_id);
				break;
			}
		}
	}
}

void Factor::measure_update() {
	double input_sum_vals = 0.0;
	double target_sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)this->factor_history.size(); h_index++) {
		input_sum_vals += this->factor_history[h_index];
		target_sum_vals += this->target_val_history[h_index];
	}
	double input_average = input_sum_vals / (double)this->factor_history.size();
	double target_average = target_sum_vals / (double)this->factor_history.size();

	double input_sum_variances = 0.0;
	double target_sum_variances = 0.0;
	for (int h_index = 0; h_index < (int)this->factor_history.size(); h_index++) {
		input_sum_variances += (this->factor_history[h_index] - input_average)
			* (this->factor_history[h_index] - input_average);
		target_sum_variances += (this->target_val_history[h_index] - target_average)
			* (this->target_val_history[h_index] - target_average);
	}
	double input_standard_deviation = sqrt(input_sum_variances / (double)this->factor_history.size());
	double target_standard_deviation = sqrt(target_sum_variances / (double)this->factor_history.size());

	this->average = input_average;
	this->standard_deviation = input_standard_deviation;

	if (input_standard_deviation < MIN_STANDARD_DEVIATION
			|| target_standard_deviation < MIN_STANDARD_DEVIATION) {
		this->pcc = 0.0;
	} else {
		double sum_covariance = 0.0;
		for (int h_index = 0; h_index < (int)this->factor_history.size(); h_index++) {
			sum_covariance += (this->factor_history[h_index] - input_average)
				* (this->target_val_history[h_index] - target_average);
		}
		double covariance = sum_covariance / (double)this->factor_history.size();

		this->pcc = covariance / input_standard_deviation / target_standard_deviation;
	}

	this->factor_history.clear();
	this->target_val_history.clear();
}

void Factor::save(ofstream& output_file) {
	output_file << this->inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		this->inputs[i_index].save(output_file);
	}

	this->network->save(output_file);

	output_file << this->pcc << endl;
	output_file << this->num_success << endl;
	output_file << this->num_failure << endl;
	output_file << this->num_selected << endl;

	output_file << this->average << endl;
	output_file << this->standard_deviation << endl;
}

void Factor::load(ifstream& input_file,
				  Solution* parent_solution) {
	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		this->inputs.push_back(Input(input_file,
									 parent_solution));
	}

	this->network = new Network(input_file);

	string pcc_line;
	getline(input_file, pcc_line);
	this->pcc = stod(pcc_line);

	string num_success_line;
	getline(input_file, num_success_line);
	this->num_success = stoi(num_success_line);

	string num_failure_line;
	getline(input_file, num_failure_line);
	this->num_failure = stoi(num_failure_line);

	string num_selected_line;
	getline(input_file, num_selected_line);
	this->num_selected = stoi(num_selected_line);

	// temp
	cout << "this->pcc: " << this->pcc << endl;
	cout << "this->num_success: " << this->num_success << endl;
	cout << "this->num_failure: " << this->num_failure << endl;
	cout << "this->num_selected: " << this->num_selected << endl;
	cout << "this->inputs.size(): " << this->inputs.size() << endl;

	string average_line;
	getline(input_file, average_line);
	this->average = stod(average_line);

	string standard_deviation_line;
	getline(input_file, standard_deviation_line);
	this->standard_deviation = stod(standard_deviation_line);
}
