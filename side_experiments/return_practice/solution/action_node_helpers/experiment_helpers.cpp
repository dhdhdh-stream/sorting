#include "action_node.h"

#include "world_model_helpers.h"

using namespace std;

void ActionNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 ExperimentRun& run) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	run.node_histories[this->id] = history;

	run.action_histories.push_back(this->action);

	action_helper(this->action,
				  run.state,
				  run.wrapper);

	action = this->action;
	is_next = true;

	run.node_context = this->next_node;
}
