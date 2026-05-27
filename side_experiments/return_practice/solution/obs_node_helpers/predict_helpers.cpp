#include "obs_node.h"

#include "experiment.h"

using namespace std;

void ObsNode::predict_step(PredictRun* run) {
	run->node_context = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->predict_activate(run);
	}
}
