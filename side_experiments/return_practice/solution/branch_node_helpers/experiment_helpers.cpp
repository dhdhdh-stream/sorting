#include "branch_node.h"

#include "experiment.h"
#include "network.h"

using namespace std;

void BranchNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 ExperimentRun& run) {
	BranchNodeHistory* history = new BranchNodeHistory(this);
	run.node_histories[this->id] = history;

	vector<double> inputs(this->input_indexes.size());
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		inputs[i_index] = run.state[this->input_indexes[i_index]];
	}
	this->original_network->activate(inputs);
	this->branch_network->activate(inputs);

	if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
		history->is_branch = true;

		run.node_context = this->branch_next_node;

		if (this->branch_experiment != NULL) {
			this->branch_experiment->experiment_activate(run);
		}
	} else {
		history->is_branch = false;

		run.node_context = this->original_next_node;

		if (this->original_experiment != NULL) {
			this->original_experiment->experiment_activate(run);
		}
	}
}
