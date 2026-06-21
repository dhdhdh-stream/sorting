#include "action_node.h"

#include "abstract_experiment.h"
#include "experiment_run.h"
#include "world_model_helpers.h"

using namespace std;

void ActionNode::experiment_step(int& action,
								 bool& is_next,
								 ExperimentRun* run) {
	run->action_histories.push_back(this->action);

	action_helper_w_history(this->action,
							run);

	action = this->action;
	is_next = true;
}

void ActionNode::experiment_step_start(ExperimentRun* run) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)run->node_histories.size();
	run->node_histories[this->id] = history;

	history->state = run->state;

	run->node_context = this->next_node;

	if (this->experiment != NULL
			&& run->should_force) {
		this->experiment->experiment_activate(run);
	}
}
