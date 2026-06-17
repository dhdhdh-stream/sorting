#include "action_node.h"

#include "predict_run.h"
#include "world_model_helpers.h"

using namespace std;

void ActionNode::predict_step(PredictRun* run) {
	action_helper(this->action,
				  run->state,
				  run->wrapper);

	predict_helper(run->state,
				   run->wrapper);

	run->node_context = this->next_node;
}
