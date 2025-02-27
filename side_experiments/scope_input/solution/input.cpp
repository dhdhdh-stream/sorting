#include "input.h"

#include <iostream>

#include "scope.h"
#include "solution.h"

using namespace std;

Input::Input() {
	// do nothing
}

Input::Input(Input& original,
			 Solution* parent_solution) {
	this->type = original.type;

	for (int l_index = 0; l_index < (int)original.scope_context.size(); l_index++) {
		this->scope_context.push_back(parent_solution->scopes[original.scope_context[l_index]->id]);
	}
	this->node_context = original.node_context;
	this->factor_index = original.factor_index;
	this->obs_index = original.obs_index;

	this->input_index = original.input_index;
}

Input::Input(ifstream& input_file,
			 Solution* parent_solution) {
	string type_line;
	getline(input_file, type_line);
	this->type = stoi(type_line);

	string context_size_line;
	getline(input_file, context_size_line);
	int context_size = stoi(context_size_line);
	for (int l_index = 0; l_index < context_size; l_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		this->scope_context.push_back(parent_solution->scopes[stoi(scope_id_line)]);

		string node_id_line;
		getline(input_file, node_id_line);
		this->node_context.push_back(stoi(node_id_line));
	}

	string factor_index_line;
	getline(input_file, factor_index_line);
	this->factor_index = stoi(factor_index_line);

	string obs_index_line;
	getline(input_file, obs_index_line);
	this->obs_index = stoi(obs_index_line);

	string input_index_line;
	getline(input_file, input_index_line);
	this->input_index = stoi(input_index_line);
}

bool Input::operator==(const Input& rhs) {
	return this->type == rhs.type
		&& this->scope_context == rhs.scope_context
		&& this->node_context == rhs.node_context
		&& this->factor_index == rhs.factor_index
		&& this->obs_index == rhs.obs_index
		&& this->input_index == rhs.input_index;
}

void Input::save(ofstream& output_file) {
	output_file << this->type << endl;

	output_file << this->scope_context.size() << endl;
	for (int l_index = 0; l_index < (int)this->scope_context.size(); l_index++) {
		output_file << this->scope_context[l_index]->id << endl;
		output_file << this->node_context[l_index] << endl;
	}
	output_file << this->factor_index << endl;
	output_file << this->obs_index << endl;

	output_file << this->input_index << endl;
}
