#include "force_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "experiment_run.h"
#include "globals.h"
#include "network.h"
#include "solution.h"
#include "start_node.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 10;
#else
const int EXPLORE_ITERS = 400;
#endif /* MDEBUG */

void ForceExperiment::explore_experiment_activate(ExperimentRun* run) {
	run->force_experiment_history->hit_branch = true;

	this->original_network->activate(run->state);
	this->curr_existing_predicted = this->original_network->output->acti_vals[0];

	AbstractNode* next_node;
	switch (node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)node_context;
			next_node = start_node->next_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node_context;
			next_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node_context;
			if (is_branch) {
				next_node = branch_node->branch_next_node;
			} else {
				next_node = branch_node->original_next_node;
			}
		}
		break;
	}
	vector<AbstractNode*> possible_exits;
	run->wrapper->solution->random_exit_activate(
		next_node,
		possible_exits);
	geometric_distribution<int> exit_distribution(0.1);
	int random_index;
	while (true) {
		random_index = exit_distribution(generator);
		if (random_index < (int)possible_exits.size()) {
			break;
		}
	}
	this->curr_exit_next_node = possible_exits[random_index];

	int new_num_steps;
	geometric_distribution<int> geo_distribution(0.3);
	if (random_index == 0) {
		new_num_steps = 1 + geo_distribution(generator);
	} else {
		new_num_steps = geo_distribution(generator);
	}

	uniform_int_distribution<int> action_distribution(0, 3);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		this->curr_actions.push_back(action_distribution(generator));
	}

	ForceExperimentState* new_experiment_state = new ForceExperimentState(this);
	new_experiment_state->step_index = 0;
	run->experiment_context = new_experiment_state;
}

void ForceExperiment::explore_experiment_step(int& action,
											  bool& is_next,
											  ExperimentRun* run) {
	ForceExperimentState* state = (ForceExperimentState*)run->experiment_context;
	if (state->step_index >= (int)this->curr_actions.size()) {
		run->node_context = this->curr_exit_next_node;

		delete run->experiment_context;
		run->experiment_context = NULL;
	} else {
		run->action_histories.push_back(this->curr_actions[state->step_index]);

		action_helper(this->curr_actions[state->step_index],
					  run->state,
					  run->wrapper);

		action = this->curr_actions[state->step_index];
		is_next = true;

		state->step_index++;
	}
}

void ForceExperiment::explore_backprop(double target_val,
									   ForceExperimentHistory* history,
									   Wrapper* wrapper) {
	if (history->hit_branch) {
		double curr_surprise = target_val - this->curr_existing_predicted;

		#if defined(MDEBUG) && MDEBUG
		if (curr_surprise > this->best_surprise || true) {
		#else
		if (curr_surprise > this->best_surprise) {
		#endif /* MDEBUG */
			this->best_surprise = curr_surprise;
			this->best_actions = this->curr_actions;
			this->best_exit_next_node = this->curr_exit_next_node;
		}

		this->curr_actions.clear();

		this->state_iter++;
		if (this->state_iter >= EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (this->best_surprise > 0.0 || rand()%2 == 0) {
			#else
			if (this->best_surprise > 0.0) {
			#endif /* MDEBUG */
				this->state = FORCE_EXPERIMENT_STATE_TRAIN_NEW;
				this->state_iter = 0;
			} else {
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
				wrapper->force_experiment = NULL;
				delete this;
			}
		}
	}
}
