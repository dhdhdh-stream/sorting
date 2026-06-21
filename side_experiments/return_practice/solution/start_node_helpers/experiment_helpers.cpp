#include "start_node.h"

#include "abstract_experiment.h"
#include "constants.h"
#include "crazy.h"
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

	if (this->experiment != NULL
			&& run->run_type == RUN_TYPE_FORCE) {
		this->experiment->experiment_activate(run);
	} else if (run->run_type == RUN_TYPE_CRAZY) {
		create_crazy_helper(this,
								false,
								run);
	}
}
