#include "compare_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "experiment_run.h"
#include "network.h"
#include "start_node.h"
#include "utilities.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

void CompareExperiment::measure_new_experiment_activate(
		ExperimentRun* run) {
	bool is_branch;
	this->original_network->activate(run->state);
	this->branch_network->activate(run->state);
	if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
		is_branch = true;
	} else {
		is_branch = false;
	}

	#if defined(MDEBUG) && MDEBUG
	if (run->wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run->wrapper->curr_run_seed = xorshift(run->wrapper->curr_run_seed);
	#endif /* MDEBUG */

	if (is_branch) {
		run->compare_experiment_history->hit_branch = true;

		CompareExperimentState* new_experiment_state = new CompareExperimentState(this);
		new_experiment_state->step_index = 0;
		run->experiment_context = new_experiment_state;
	}
}

void CompareExperiment::measure_new_experiment_step(
		int& action,
		bool& is_next,
		ExperimentRun* run) {
	CompareExperimentState* state = (CompareExperimentState*)run->experiment_context;
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

void CompareExperiment::measure_new_backprop(
		double target_val,
		ExperimentRun* run,
		Wrapper* wrapper) {
	if (run->compare_experiment_history->hit_branch) {
		this->sum_scores += target_val;

		this->state_iter++;
		if (this->state_iter >= MEASURE_NUM_ITERS) {
			double new_average_score = this->sum_scores / (double)MEASURE_NUM_ITERS;

			cout << "this->existing_average_score: " << this->existing_average_score << endl;
			cout << "new_average_score: " << new_average_score << endl;
			cout << "predicted_existing_average: " << predicted_existing_average << endl;
			cout << "predicted_new_average: " << predicted_new_average << endl;

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
			wrapper->compare_experiment = NULL;
			delete this;
		}
	}
}
