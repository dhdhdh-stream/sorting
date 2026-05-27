#include "obs_node.h"

#include "constants.h"
#include "experiment.h"
#include "world_model_helpers.h"

using namespace std;

void ObsNode::experiment_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  ExperimentRun* run) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	run->node_histories[this->id] = history;

	if (this->state_history.size() < STATE_HISTORY_NUM_SAVE) {
		this->state_history.push_back(run->state);
	} else {
		this->state_history[this->state_history_index] = run->state;
	}
	this->state_history_index++;
	if (this->state_history_index >= STATE_HISTORY_NUM_SAVE) {
		this->state_history_index = 0;
	}

	run->node_context = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->experiment_activate(run);
	}
}
