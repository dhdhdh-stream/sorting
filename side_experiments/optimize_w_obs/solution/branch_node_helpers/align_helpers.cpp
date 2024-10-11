#include "branch_node.h"

#include "network.h"

using namespace std;

void BranchNode::align_activate(AbstractNode*& curr_node,
								vector<ContextLayer>& context) {
	vector<double> input_vals(this->inputs.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		map<pair<pair<vector<int>,vector<int>>,int>, double>::iterator it =
			context.back().obs_history.find(this->inputs[i_index]);
		if (it != context.back().obs_history.end()) {
			input_vals[i_index] = it->second;
		}
	}
	this->network->activate(input_vals);

	bool is_branch;
	if (this->network->output->acti_vals[0] >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}
}
