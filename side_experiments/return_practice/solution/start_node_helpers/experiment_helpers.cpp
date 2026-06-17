#include "start_node.h"

#include "abstract_experiment.h"
#include "constants.h"
#include "experiment_run.h"

using namespace std;

void StartNode::experiment_step(int& action,
								bool& is_next,
								ExperimentRun* run) {
	// unreachable
}

void StartNode::experiment_step_start(ExperimentRun* run) {
	StartNodeHistory* history = new StartNodeHistory(this);
	history->index = (int)run->node_histories.size();
	run->node_histories[this->id] = history;

	history->state = run->state;

	run->node_context = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->experiment_activate(run);
	}
}
