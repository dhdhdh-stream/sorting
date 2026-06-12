#include "crazy.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "experiment_run.h"
#include "start_node.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

Crazy::~Crazy() {
	switch (this->node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)this->node_context;
			start_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			action_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				branch_node->branch_experiment = NULL;
			} else {
				branch_node->original_experiment = NULL;
			}
		}
		break;
	}
}

void Crazy::experiment_activate(ExperimentRun* run) {
	run->wrapper->hit_crazy = true;

	CrazyState* new_experiment_state = new CrazyState(this);
	new_experiment_state->step_index = 0;
	run->experiment_context = new_experiment_state;
}

void Crazy::experiment_step(int& action,
							bool& is_next,
							ExperimentRun* run) {
	CrazyState* state = (CrazyState*)run->experiment_context;
	if (state->step_index >= (int)this->actions.size()) {
		run->node_context = this->exit_next_node;

		delete run->experiment_context;
		run->experiment_context = NULL;
	} else {
		run->action_histories.push_back(this->actions[state->step_index]);

		action_helper(this->actions[state->step_index],
					  run->state,
					  run->wrapper);

		action = this->actions[state->step_index];
		is_next = true;

		state->step_index++;
	}
}

void Crazy::predict_activate(PredictRun* run) {
	// do nothing
}

CrazyState::CrazyState(Crazy* experiment) {
	this->experiment = experiment;
}
