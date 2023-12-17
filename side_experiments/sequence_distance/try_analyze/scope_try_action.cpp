#include "scope_try_action.h"



double ScopeTryAction::compare(ScopeTryAction* original) {


	vector<AbstractNode*> shared;
	for (int on_index = 0; on_index < (int)original->original_nodes.size(); on_index++) {
		bool is_match = false;
		for (int nn_index = 0; nn_index < (int)this->original_nodes.size(); nn_index++) {
			if (this->original_nodes[nn_index] == original->original_nodes[on_index]) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			shared.push_back(original->original_nodes[on_index]);
		}
	}

	set<AbstractNode*> all;
	for (int n_index = 0; n_index < (int)original->original_nodes.size(); n_index++) {
		all.insert(original->original_nodes[n_index]);
	}
	for (int n_index = 0; n_index < (int)this->original_nodes.size(); n_index++) {
		all.insert(this->original_nodes[n_index])
	}

	double node_similarity = ((double)shared.size())/((double)all.size());
}
