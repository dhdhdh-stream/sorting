#include "action_node.h"

#include "experiment.h"
#include "experiment_run.h"
#include "world_model_helpers.h"

using namespace std;

void ActionNode::experiment_step(int& action,
								 bool& is_next,
								 ExperimentRun* run) {
	for (int e_index = 0; e_index < (int)this->exit_experiments.size(); e_index++) {
		this->exit_experiments[e_index]->experiment_exit(run);
	}

	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)run->node_histories.size();
	run->node_histories[this->id] = history;

	run->action_histories.push_back(this->action);

	action_helper(this->action,
				  run->state,
				  run->wrapper);

	action = this->action;
	is_next = true;
}

void ActionNode::experiment_step_start(ExperimentRun* run) {
	run->node_context = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->experiment_activate(run);
	}
}
