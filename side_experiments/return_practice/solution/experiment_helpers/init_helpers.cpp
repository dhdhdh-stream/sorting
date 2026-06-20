/**
 * - need long-trained network for end network
 *   - need long-trained to predict crazy correctly
 * 
 * TODO: instead of predicting until end, predict until signal end of scope
 */

#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "compare_experiment.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "start_node.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_EXPLORE = 5;
#else
const int NUM_EXPLORE = 1000;
#endif /* MDEBUG */

void init_experiment_helper(AbstractNode* node_context,
							bool is_branch,
							AbstractNode* exit_next_node,
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

	vector<vector<double>> train_existing_state(EXPERIMENT_NUM_SAMPLES);
	vector<double> train_existing_predicted(EXPERIMENT_NUM_SAMPLES);
	for (int iter_index = 0; iter_index < EXPERIMENT_NUM_SAMPLES; iter_index++) {
		int start_index = start_distribution(generator);
		train_existing_state[iter_index] = state_history[start_index];
		train_existing_predicted[iter_index] = predict_helper(state_history[start_index],
															  next_node,
															  wrapper);
	}

	int num_verify = VERIFICATION_RATIO * (double)EXPERIMENT_NUM_SAMPLES;
	int num_train = EXPERIMENT_NUM_SAMPLES - num_verify;

	uniform_int_distribution<int> sample_distribution(0, num_train-1);

	Network* original_network = new Network(train_existing_state[0].size());
	double original_hidden_1_average_max_update = 0.0;
	double original_hidden_2_average_max_update = 0.0;
	double original_hidden_3_average_max_update = 0.0;
	double original_output_average_max_update = 0.0;
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = sample_distribution(generator);
		original_network->activate(train_existing_state[index]);
		double error = train_existing_predicted[index] - original_network->output->acti_vals[0];
		original_network->init_backprop(error,
										original_hidden_1_average_max_update,
										original_hidden_2_average_max_update,
										original_hidden_3_average_max_update,
										original_output_average_max_update);
	}

	vector<int> best_actions;
	double best_surprise = numeric_limits<double>::lowest();
	for (int e_index = 0; e_index < NUM_EXPLORE; e_index++) {
		int start_index = start_distribution(generator);

		vector<double> state = state_history[start_index];

		original_network->activate(state);
		double existing_predicted = original_network->output->acti_vals[0];

		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.3);
		if (exit_next_node == next_node) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}

		vector<int> curr_actions;
		uniform_int_distribution<int> action_distribution(0, wrapper->num_actions-1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			curr_actions.push_back(action_distribution(generator));
		}

		for (int a_index = 0; a_index < (int)curr_actions.size(); a_index++) {
			action_helper(curr_actions[a_index],
						  state,
						  wrapper);

			predict_helper(state,
						   wrapper);
		}
		double predicted = predict_helper(state,
										  exit_next_node,
										  wrapper);

		double curr_surprise = predicted - existing_predicted;
		if (curr_surprise > best_surprise) {
			best_actions = curr_actions;
			best_surprise = curr_surprise;
		}
	}

	vector<vector<double>> train_new_state(EXPERIMENT_NUM_SAMPLES);
	vector<double> train_new_predicted(EXPERIMENT_NUM_SAMPLES);
	for (int iter_index = 0; iter_index < EXPERIMENT_NUM_SAMPLES; iter_index++) {
		int start_index = start_distribution(generator);

		train_new_state[iter_index] = state_history[start_index];

		vector<double> state = state_history[start_index];
		for (int a_index = 0; a_index < (int)best_actions.size(); a_index++) {
			action_helper(best_actions[a_index],
						  state,
						  wrapper);

			predict_helper(state,
						   wrapper);
		}
		train_new_predicted[iter_index] = predict_helper(state,
														 exit_next_node,
														 wrapper);
	}

	Network* branch_network = new Network(train_new_state[0].size());
	double branch_hidden_1_average_max_update = 0.0;
	double branch_hidden_2_average_max_update = 0.0;
	double branch_hidden_3_average_max_update = 0.0;
	double branch_output_average_max_update = 0.0;
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = sample_distribution(generator);
		branch_network->activate(train_new_state[index]);
		double error = train_new_predicted[index] - branch_network->output->acti_vals[0];
		branch_network->init_backprop(error,
									  branch_hidden_1_average_max_update,
									  branch_hidden_2_average_max_update,
									  branch_hidden_3_average_max_update,
									  branch_output_average_max_update);
	}

	double existing_sum_vals = 0.0;
	int existing_count = 0;
	for (int h_index = num_train; h_index < (int)train_existing_state.size(); h_index++) {
		original_network->activate(train_existing_state[h_index]);
		branch_network->activate(train_existing_state[h_index]);
		if (branch_network->output->acti_vals[0] > original_network->output->acti_vals[0]) {
			existing_sum_vals += train_existing_predicted[h_index];
			existing_count++;
		}
	}
	double existing_average = existing_sum_vals / (double)existing_count;
	double new_sum_vals = 0.0;
	int new_count = 0;
	for (int h_index = num_train; h_index < (int)train_new_state.size(); h_index++) {
		original_network->activate(train_new_state[h_index]);
		branch_network->activate(train_new_state[h_index]);
		if (branch_network->output->acti_vals[0] > original_network->output->acti_vals[0]) {
			new_sum_vals += train_new_predicted[h_index];
			new_count++;
		}
	}
	double new_average = new_sum_vals / (double)new_count;
	double average_ratio = (existing_count + new_count) / (2.0 * num_verify);
	double predicted_local_improvement = (new_average - existing_average) * average_ratio;

	double average_instances_per_run;
	switch (node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)node_context;
			average_instances_per_run = start_node->average_instances_per_run;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node_context;
			average_instances_per_run = action_node->average_instances_per_run;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node_context;
			if (is_branch) {
				average_instances_per_run = branch_node->branch_average_instances_per_run;
			} else {
				average_instances_per_run = branch_node->original_average_instances_per_run;
			}
		}
		break;
	}

	double predicted_global_improvement = average_instances_per_run * predicted_local_improvement;

	// temp
	cout << "predicted_local_improvement: " << predicted_local_improvement << endl;
	cout << "predicted_global_improvement: " << predicted_global_improvement << endl;

	bool is_success = false;
	if (predicted_local_improvement > 0.0) {
		if (wrapper->solution->train_new_last_scores.size() >= MIN_NUM_LAST_TRACK) {
			int num_better_than = 0;
			for (list<double>::iterator it = wrapper->solution->train_new_last_scores.begin();
					it != wrapper->solution->train_new_last_scores.end(); it++) {
				if (predicted_global_improvement >= *it) {
					num_better_than++;
				}
			}

			double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->train_new_last_scores.size();

			if (num_better_than >= target_better_than) {
				is_success = true;
			}

			if (wrapper->solution->train_new_last_scores.size() >= NUM_LAST_TRACK) {
				wrapper->solution->train_new_last_scores.pop_front();
			}
			wrapper->solution->train_new_last_scores.push_back(predicted_global_improvement);
		} else {
			wrapper->solution->train_new_last_scores.push_back(predicted_global_improvement);
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if (is_success || rand()%3 != 0) {
	#else
	if (is_success) {
	#endif /* MDEBUG */
		// finalize_helper(node_context,
		// 				is_branch,
		// 				best_actions,
		// 				exit_next_node,
		// 				original_network,
		// 				branch_network,
		// 				wrapper);

		CompareExperiment* experiment = new CompareExperiment();

		experiment->node_context = node_context;
		experiment->is_branch = is_branch;
		experiment->exit_next_node = exit_next_node;

		experiment->original_network = original_network;
		experiment->branch_network = branch_network;

		experiment->actions = best_actions;

		experiment->predicted_existing_average = existing_average;
		experiment->predicted_new_average = new_average;

		experiment->sum_scores = 0.0;

		experiment->state = COMPARE_EXPERIMENT_MEASURE_EXISTING;
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
		wrapper->compare_experiment = experiment;
	} else {
		delete original_network;
		delete branch_network;
	}
}
