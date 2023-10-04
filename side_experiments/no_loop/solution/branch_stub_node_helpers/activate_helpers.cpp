#include "branch_stub_node.h"

using namespace std;

void BranchStubNode::activate(vector<ContextLayer>& context) {
	double obs_snapshot;
	if (this->was_branch) {
		obs_snapshot = -1.0;
	} else {
		obs_snapshot = 1.0;
	}

	for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
		if (this->state_is_local[n_index]) {
			map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->state_indexes[n_index]);
			if (it == context.back().local_state_vals.end()) {
				it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
			state_network->activate(obs_snapshot,
									it->second);
		} else {
			map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->state_indexes[n_index]);
			if (it != context.back().input_state_vals.end()) {
				StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				state_network->activate(obs_snapshot,
										it->second);
			}
		}
	}
}
