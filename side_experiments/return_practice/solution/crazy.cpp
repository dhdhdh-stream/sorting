#include "crazy.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "experiment_run.h"
#include "globals.h"
#include "solution.h"
#include "solution_helpers.h"
#include "start_node.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_EXPLORE = 2;
const int NUM_SIMULATE = 2;
#else
const int NUM_EXPLORE = 100;
const int NUM_SIMULATE = 10;
#endif /* MDEBUG */

void create_crazy_helper(AbstractNode* node_context,
						 bool is_branch,
						 ExperimentRun* run) {
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

	double best_sum_predicted = numeric_limits<double>::lowest();
	vector<int> best_actions;
	AbstractNode* best_exit_next_node;
	for (int explore_index = 0; explore_index < NUM_EXPLORE; explore_index++) {
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
		AbstractNode* curr_exit_next_node = possible_exits[random_index];

		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.3);
		if (random_index == 0) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}

		vector<int> curr_actions;
		uniform_int_distribution<int> action_distribution(0, run->wrapper->num_actions-1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			curr_actions.push_back(action_distribution(generator));
		}

		vector<double> state = run->state;

		double sum_predicted = 0.0;
		for (int iter_index = 0; iter_index < NUM_SIMULATE; iter_index++) {
			for (int a_index = 0; a_index < (int)curr_actions.size(); a_index++) {
				action_helper(curr_actions[a_index],
							  state,
							  run->wrapper);

				predict_helper(state,
							   run->wrapper);
			}
			double predicted = predict_helper(state,
											  curr_exit_next_node,
											  run->wrapper);
			sum_predicted += predicted;
		}

		if (sum_predicted > best_sum_predicted) {
			best_sum_predicted = sum_predicted;
			best_actions = curr_actions;
			best_exit_next_node = curr_exit_next_node;
		}
	}

	Crazy* crazy = new Crazy();
	crazy->actions = best_actions;
	crazy->exit_next_node = best_exit_next_node;

	run->crazies.push_back(crazy);

	CrazyState* new_experiment_state = new CrazyState(crazy);
	new_experiment_state->step_index = 0;
	run->experiment_context = new_experiment_state;
}

void Crazy::experiment_activate(ExperimentRun* run) {
	// unreachable
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

		// action_helper(this->actions[state->step_index],
		// 			  run->state,
		// 			  run->wrapper);
		action_helper_w_history(this->actions[state->step_index],
								run);

		action = this->actions[state->step_index];
		is_next = true;

		state->step_index++;
	}
}

CrazyState::CrazyState(Crazy* crazy) {
	this->experiment = crazy;
}
