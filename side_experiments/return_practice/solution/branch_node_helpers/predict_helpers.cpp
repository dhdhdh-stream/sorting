#include "branch_node.h"

#include "experiment.h"
#include "network.h"

using namespace std;

void BranchNode::predict_step(PredictRun* run) {
	this->original_network->activate(run->state);
	this->branch_network->activate(run->state);

	if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
		run->node_context = this->branch_next_node;

		if (this->branch_experiment != NULL) {
			this->branch_experiment->predict_activate(run);
		}
	} else {
		run->node_context = this->original_next_node;

		if (this->original_experiment != NULL) {
			this->original_experiment->predict_activate(run);
		}
	}
}
