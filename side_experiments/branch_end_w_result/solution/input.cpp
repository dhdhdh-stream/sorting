#include "input.h"

#include <iostream>

#include "scope.h"
#include "solution.h"

using namespace std;

Input::Input() {
	// do nothing
}

Input::Input(ifstream& input_file,
			 Solution* parent_solution) {
	string num_layers_line;
	getline(input_file, num_layers_line);
	int num_layers = stoi(num_layers_line);
	for (int l_index = 0; l_index < num_layers; l_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		int scope_id = stoi(scope_id_line);

		string is_external_line;
		getline(input_file, is_external_line);
		bool is_external = stoi(is_external_line);

		if (is_external) {
			this->scope_context.push_back(parent_solution->external_scopes[scope_id]);
		} else {
			this->scope_context.push_back(parent_solution->scopes[scope_id]);
		}

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
}

bool Input::operator==(const Input& rhs) const {
	return this->scope_context == rhs.scope_context
		&& this->node_context == rhs.node_context
		&& this->factor_index == rhs.factor_index
		&& this->obs_index == rhs.obs_index;
}

bool Input::operator!=(const Input& rhs) const {
	return this->scope_context != rhs.scope_context
		|| this->node_context != rhs.node_context
		|| this->factor_index != rhs.factor_index
		|| this->obs_index != rhs.obs_index;
}

bool Input::operator<(const Input& rhs) const {
	if (this->scope_context != rhs.scope_context) {
		return this->scope_context < rhs.scope_context;
	} else {
		if (this->node_context != rhs.node_context) {
			return this->node_context < rhs.node_context;
		} else {
			if (this->factor_index != rhs.factor_index) {
				return this->factor_index < rhs.factor_index;
			} else {
				if (this->obs_index != rhs.obs_index) {
					return this->obs_index < rhs.obs_index;
				} else {
					return false;
				}
			}
		}
	}
}

bool Input::operator>(const Input& rhs) const {
	if (this->scope_context != rhs.scope_context) {
		return this->scope_context > rhs.scope_context;
	} else {
		if (this->node_context != rhs.node_context) {
			return this->node_context > rhs.node_context;
		} else {
			if (this->factor_index != rhs.factor_index) {
				return this->factor_index > rhs.factor_index;
			} else {
				if (this->obs_index != rhs.obs_index) {
					return this->obs_index > rhs.obs_index;
				} else {
					return false;
				}
			}
		}
	}
}

bool Input::operator<=(const Input& rhs) const {
	if (this->scope_context != rhs.scope_context) {
		return this->scope_context < rhs.scope_context;
	} else {
		if (this->node_context != rhs.node_context) {
			return this->node_context < rhs.node_context;
		} else {
			if (this->factor_index != rhs.factor_index) {
				return this->factor_index < rhs.factor_index;
			} else {
				if (this->obs_index != rhs.obs_index) {
					return this->obs_index < rhs.obs_index;
				} else {
					return true;
				}
			}
		}
	}
}

bool Input::operator>=(const Input& rhs) const {
	if (this->scope_context != rhs.scope_context) {
		return this->scope_context > rhs.scope_context;
	} else {
		if (this->node_context != rhs.node_context) {
			return this->node_context > rhs.node_context;
		} else {
			if (this->factor_index != rhs.factor_index) {
				return this->factor_index > rhs.factor_index;
			} else {
				if (this->obs_index != rhs.obs_index) {
					return this->obs_index > rhs.obs_index;
				} else {
					return true;
				}
			}
		}
	}
}

void Input::print() {
	for (int l_index = 0; l_index < (int)this->scope_context.size(); l_index++) {
		cout << this->scope_context[l_index]->id << " " << this->node_context[l_index] << endl;
	}
	cout << this->factor_index << endl;
	cout << this->obs_index << endl;
}

void Input::save(ofstream& output_file) {
	output_file << this->scope_context.size() << endl;
	for (int l_index = 0; l_index < (int)this->scope_context.size(); l_index++) {
		output_file << this->scope_context[l_index]->id << endl;
		output_file << this->scope_context[l_index]->is_external << endl;
		output_file << this->node_context[l_index] << endl;
	}

	output_file << this->factor_index << endl;
	output_file << this->obs_index << endl;
}
