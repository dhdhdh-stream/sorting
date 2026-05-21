#include "obs_node.h"

#include "experiment.h"
#include "world_model_helpers.h"

using namespace std;

void ObsNode::experiment_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  ExperimentRun& run) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	run.node_histories[this->id] = history;

	run.obs_histories.push_back(obs);

	obs_helper(obs,
			   run.state,
			   run.wrapper);

	run.node_context = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->experiment_activate(run);
	}
}
