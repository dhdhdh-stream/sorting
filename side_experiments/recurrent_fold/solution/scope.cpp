#include "scope.h"

using namespace std;

Scope::Scope(int num_input_states,
			 int num_local_states,
			 bool is_loop,
			 Network* continue_network,
			 Network* halt_network,
			 vector<AbstractNode*> nodes) {
	this->num_input_states = num_input_states;
	this->num_local_states = num_local_states;
	this->is_loop = is_loop;
	this->continue_network = continue_network;
	this->halt_network = halt_network;
	this->nodes = nodes;
}

Scope::~Scope() {
	if (this->continue_network != NULL) {
		delete this->continue_network;
	}

	if (this->halt_network != NULL) {
		delete this->halt_network;
	}

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}
}

void Scope::activate(vector<double>& input_vals,
					 vector<vector<double>>& flat_vals,
					 double& predicted_score) {
	vector<double> local_state_vals(this->num_local_states, 0.0);

	int curr_node_index = 0;
	while (true) {
		if (curr_node_index == -1) {
			break;
		}

		int next_node_index = this->nodes[curr_node_index]->activate(
			input_vals,
			local_state_vals,
			flat_vals,
			predicted_score);
		curr_node_index = next_node_index;
	}
	// TODO: when exploring, at end, explore (with the possibility to kick to outside?)
	// advantages of exploring within is still have access to state
	// kicking outside can generate different sequences
	// so treat end and everywhere else the same
	// don't branch at start
	// have no information at start, so would essentially be randomly trying stuff
}
