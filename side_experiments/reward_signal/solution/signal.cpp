#include "signal.h"

#include <iostream>

using namespace std;

Signal::Signal() {
	// do nothing
}

Signal::Signal(ifstream& input_file,
			   Solution* parent_solution) {
	string match_factor_index_line;
	getline(input_file, match_factor_index_line);
	this->match_factor_index = stoi(match_factor_index_line);

	string score_average_val_line;
	getline(input_file, score_average_val_line);
	this->score_average_val = stod(score_average_val_line);

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		this->score_inputs.push_back(Input(input_file,
										   parent_solution));

		string input_average_line;
		getline(input_file, input_average_line);
		this->score_input_averages.push_back(stod(input_average_line));

		string input_standard_deviation_line;
		getline(input_file, input_standard_deviation_line);
		this->score_input_standard_deviations.push_back(stod(input_standard_deviation_line));

		string weight_line;
		getline(input_file, weight_line);
		this->score_weights.push_back(stod(weight_line));
	}
}

void Signal::clean_inputs(Scope* scope,
						  int node_id) {
	for (int i_index = (int)this->score_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->score_inputs[i_index].scope_context.size(); l_index++) {
			if (this->score_inputs[i_index].scope_context[l_index] == scope
					&& this->score_inputs[i_index].node_context[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->score_inputs.erase(this->score_inputs.begin() + i_index);
			this->score_input_averages.erase(this->score_input_averages.begin() + i_index);
			this->score_input_standard_deviations.erase(this->score_input_standard_deviations.begin() + i_index);
			this->score_weights.erase(this->score_weights.begin() + i_index);
		}
	}
}

void Signal::clean_inputs(Scope* scope) {
	for (int i_index = (int)this->score_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->score_inputs[i_index].scope_context.size(); l_index++) {
			if (this->score_inputs[i_index].scope_context[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->score_inputs.erase(this->score_inputs.begin() + i_index);
			this->score_input_averages.erase(this->score_input_averages.begin() + i_index);
			this->score_input_standard_deviations.erase(this->score_input_standard_deviations.begin() + i_index);
			this->score_weights.erase(this->score_weights.begin() + i_index);
		}
	}
}

void Signal::replace_obs_node(Scope* scope,
							  int original_node_id,
							  int new_node_id) {
	for (int i_index = 0; i_index < (int)this->score_inputs.size(); i_index++) {
		if (this->score_inputs[i_index].scope_context.back() == scope
				&& this->score_inputs[i_index].node_context.back() == original_node_id) {
			this->score_inputs[i_index].node_context.back() = new_node_id;
		}
	}
}

void Signal::save(ofstream& output_file) {
	output_file << this->match_factor_index << endl;

	output_file << this->score_average_val << endl;

	output_file << this->score_inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->score_inputs.size(); i_index++) {
		this->score_inputs[i_index].save(output_file);
		output_file << this->score_input_averages[i_index] << endl;
		output_file << this->score_input_standard_deviations[i_index] << endl;
		output_file << this->score_weights[i_index] << endl;
	}
}
