#include "signal_input.h"

#include <iostream>

#include "scope.h"
#include "solution.h"

using namespace std;

SignalInput::SignalInput() {
	// do nothing
}

SignalInput::SignalInput(SignalInput& original,
						 Solution* parent_solution) {
	this->is_pre = original.is_pre;
	for (int s_index = 0; s_index < (int)original.scope_context.size(); s_index++) {
		this->scope_context.push_back(parent_solution->scopes[original.scope_context[s_index]->id]);
	}
	this->node_context = original.node_context;
	this->obs_index = original.obs_index;
}

SignalInput::SignalInput(ifstream& input_file,
						 Solution* parent_solution) {
	string num_layers_line;
	getline(input_file, num_layers_line);
	int num_layers = stoi(num_layers_line);
	for (int l_index = 0; l_index < num_layers; l_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		this->scope_context.push_back(parent_solution->scopes[stoi(scope_id_line)]);

		string node_id_line;
		getline(input_file, node_id_line);
		this->node_context.push_back(stoi(node_id_line));
	}

	string obs_index_line;
	getline(input_file, obs_index_line);
	this->obs_index = stoi(obs_index_line);
}

bool SignalInput::operator==(const SignalInput& rhs) const {
	return this->scope_context == rhs.scope_context
		&& this->node_context == rhs.node_context
		&& this->obs_index == rhs.obs_index;
}

bool SignalInput::operator!=(const SignalInput& rhs) const {
	return this->scope_context != rhs.scope_context
		|| this->node_context != rhs.node_context
		|| this->obs_index != rhs.obs_index;
}

bool SignalInput::operator<(const SignalInput& rhs) const {
	if (this->scope_context != rhs.scope_context) {
		return this->scope_context < rhs.scope_context;
	} else {
		if (this->node_context != rhs.node_context) {
			return this->node_context < rhs.node_context;
		} else {
			if (this->obs_index != rhs.obs_index) {
				return this->obs_index < rhs.obs_index;
			} else {
				return false;
			}
		}
	}
}

bool SignalInput::operator>(const SignalInput& rhs) const {
	if (this->scope_context != rhs.scope_context) {
		return this->scope_context > rhs.scope_context;
	} else {
		if (this->node_context != rhs.node_context) {
			return this->node_context > rhs.node_context;
		} else {
			if (this->obs_index != rhs.obs_index) {
				return this->obs_index > rhs.obs_index;
			} else {
				return false;
			}
		}
	}
}

bool SignalInput::operator<=(const SignalInput& rhs) const {
	if (this->scope_context != rhs.scope_context) {
		return this->scope_context < rhs.scope_context;
	} else {
		if (this->node_context != rhs.node_context) {
			return this->node_context < rhs.node_context;
		} else {
			if (this->obs_index != rhs.obs_index) {
				return this->obs_index < rhs.obs_index;
			} else {
				return true;
			}
		}
	}
}

bool SignalInput::operator>=(const SignalInput& rhs) const {
	if (this->scope_context != rhs.scope_context) {
		return this->scope_context > rhs.scope_context;
	} else {
		if (this->node_context != rhs.node_context) {
			return this->node_context > rhs.node_context;
		} else {
			if (this->obs_index != rhs.obs_index) {
				return this->obs_index > rhs.obs_index;
			} else {
				return true;
			}
		}
	}
}

void SignalInput::print() {
	for (int l_index = 0; l_index < (int)this->scope_context.size(); l_index++) {
		cout << this->scope_context[l_index]->id << " " << this->node_context[l_index] << endl;
	}
	cout << this->obs_index << endl;
}

void SignalInput::save(ofstream& output_file) {
	output_file << this->scope_context.size() << endl;
	for (int l_index = 0; l_index < (int)this->scope_context.size(); l_index++) {
		output_file << this->scope_context[l_index]->id << endl;
		output_file << this->node_context[l_index] << endl;
	}

	output_file << this->obs_index << endl;
}
