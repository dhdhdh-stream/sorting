#include "factor.h"

#include <iostream>

#include "branch_node.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"

using namespace std;

Factor::Factor() {
	// do nothing
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

void Factor::replace_obs_node(Scope* scope,
							  int original_node_id,
							  int new_node_id,
							  int index) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		if (this->inputs[i_index].scope_context.back() == scope
				&& this->inputs[i_index].node_context.back() == original_node_id) {
			this->inputs[i_index].node_context.back() = new_node_id;

			if (this->inputs[i_index].scope_context.size() == 1) {
				scope->nodes[new_node_id]->impacted_factors.push_back(index);
			}
		}
	}
}

void Factor::save(ofstream& output_file) {
	output_file << this->inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		this->inputs[i_index].save(output_file);
	}

	this->network->save(output_file);

	output_file << this->is_meaningful << endl;
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

	string is_meaningful_line;
	getline(input_file, is_meaningful_line);
	this->is_meaningful = stoi(is_meaningful_line);
}

void Factor::link(int index) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		if (this->inputs[i_index].scope_context.size() == 1
				&& this->inputs[i_index].factor_index != -1) {
			Scope* scope = this->inputs[i_index].scope_context[0];
			Factor* factor = scope->factors[this->inputs[i_index].factor_index];
			factor->impacted_factors.push_back(index);
		} else {
			Scope* scope = this->inputs[i_index].scope_context[0];
			AbstractNode* node = scope->nodes[this->inputs[i_index].node_context[0]];
			node->impacted_factors.push_back(index);
		}
	}
}
