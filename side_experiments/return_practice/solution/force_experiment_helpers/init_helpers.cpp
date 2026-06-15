#include "force_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "solution_helpers.h"
#include "start_node.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_SIMULATE = 40;
#else
const int NUM_SIMULATE = 4000;
#endif /* MDEBUG */

void init_force_experiment_helper(AbstractNode* node_context,
								  bool is_branch,
								  Wrapper* wrapper) {
	vector<vector<double>> state_history;
	switch (node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)node_context;
			state_history = start_node->state_history;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node_context;
			state_history = action_node->state_history;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node_context;
			if (is_branch) {
				state_history = branch_node->branch_state_history;
			} else {
				state_history = branch_node->original_state_history;
			}
		}
		break;
	}
	uniform_int_distribution<int> start_distribution(0, state_history.size()-1);

	uniform_int_distribution<int> train_distribution(0, NUM_SIMULATE-1);

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

	vector<vector<double>> train_existing_state(NUM_SIMULATE);
	vector<double> train_existing_predicted(NUM_SIMULATE);
	for (int iter_index = 0; iter_index < NUM_SIMULATE; iter_index++) {
		int start_index = start_distribution(generator);
		train_existing_state[iter_index] = state_history[start_index];
		train_existing_predicted[iter_index] = predict_helper(state_history[start_index],
															  next_node,
															  wrapper);
	}

	Network* original_network = new Network(state_history[0].size());
	double original_hidden_1_average_max_update = 0.0;
	double original_hidden_2_average_max_update = 0.0;
	double original_hidden_3_average_max_update = 0.0;
	double original_output_average_max_update = 0.0;
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = train_distribution(generator);
		original_network->activate(train_existing_state[index]);
		double error = train_existing_predicted[index] - original_network->output->acti_vals[0];
		original_network->init_backprop(error,
										original_hidden_1_average_max_update,
										original_hidden_2_average_max_update,
										original_hidden_3_average_max_update,
										original_output_average_max_update);
	}

	ForceExperiment* experiment = new ForceExperiment();

	experiment->node_context = node_context;
	experiment->is_branch = is_branch;

	experiment->original_network = original_network;

	experiment->best_surprise = 0.0;

	experiment->state = FORCE_EXPERIMENT_STATE_EXPLORE;
	experiment->state_iter = 0;

	switch (node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)node_context;
			start_node->experiment = experiment;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node_context;
			action_node->experiment = experiment;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node_context;
			if (is_branch) {
				branch_node->branch_experiment = experiment;
			} else {
				branch_node->original_experiment = experiment;
			}
		}
		break;
	}
	wrapper->force_experiment = experiment;

	// temp
	cout << "force_experiment" << endl;
}
