#include "end_node.h"

#include "constants.h"
#include "experiment.h"
#include "experiment_run.h"

using namespace std;

void EndNode::experiment_step(int& action,
							  bool& is_next,
							  ExperimentRun* run) {
	for (int e_index = 0; e_index < (int)this->exit_experiments.size(); e_index++) {
		this->exit_experiments[e_index]->experiment_exit(run);
	}

	EndNodeHistory* history = new EndNodeHistory(this);
	history->index = (int)run->node_histories.size();
	run->node_histories[this->id] = history;

	run->node_context = NULL;
}

void EndNode::experiment_step_start(ExperimentRun* run) {
	// unreachable
}
