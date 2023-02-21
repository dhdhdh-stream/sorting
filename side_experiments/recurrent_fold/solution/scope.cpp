#include "scope.h"

using namespace std;

Scope::Scope(int num_input_states,
			 int num_local_states,
			 bool is_loop,
			 Network* continue_network,
			 Network* halt_network,
			 vector<Network*> score_networks,
			 vector<bool> is_fold,
			 vector<ScopePath*> branches,
			 vector<Fold*> folds,
			 vector<int> num_travelled) {
	this->num_input_states = num_input_states;
	this->num_local_states = num_local_states;
	this->is_loop = is_loop;
	this->continue_network = continue_network;
	this->halt_network = halt_network;
	this->score_networks = score_networks;
	this->is_fold = is_fold;
	this->branches = branches;
	this->folds = folds;
	this->num_travelled = num_travelled;
}

Scope::~Scope() {
	if (this->continue_network != NULL) {
		delete this->continue_network;
	}

	if (this->halt_network != NULL) {
		delete this->halt_network;
	}

	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		if (this->score_networks[b_index] != NULL) {
			delete this->score_networks[b_index];
		}

		if (this->branches[b_index] != NULL) {
			delete this->branches[b_index];
		}

		if (this->folds[b_index] != NULL) {
			delete this->folds[b_index];
		}
	}
}

void Scope::activate(vector<double>& input_vals,
					 vector<vector<double>>& flat_vals,
					 double& predicted_score) {
	if (is_loop) {
		// while (true) {
		// 	vector<double> local_state_vals(this->num_local_states, 0.0);

		// }
	} else {
		if (this->branches.size() == 1) {
			this->branches[0]->activate(input_vals,
										flat_vals,
										predicted_score);
		} else {

		}
	}
}
