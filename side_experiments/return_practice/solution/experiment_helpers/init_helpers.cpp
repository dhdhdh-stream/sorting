#include "experiment.h"

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "solution.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NUM_SAMPLES = 10;
#else
const int TRAIN_NUM_SAMPLES = 1000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_EXPLORE = 5;
#else
const int NUM_EXPLORE = 500;
#endif /* MDEBUG */

void init_experiment_helper(AbstractNode* node_context,
							bool is_branch,
							Wrapper* wrapper) {
	uniform_int_distribution<int> train_distribution(0, TRAIN_NUM_SAMPLES-1);

	vector<vector<double>> existing_state;
	vector<double> existing_predicted;
	for (int h_index = 0; h_index < TRAIN_NUM_SAMPLES; h_index++) {
		switch (node_context->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node_context;
				uniform_int_distribution<int> history_distribution(0, obs_node->state_history.size()-1);
				int history_index = history_distribution(generator);
				double predicted = predict_helper(node_context,
												  is_branch,
												  obs_node->state_history[history_index],
												  wrapper);
				existing_state.push_back(obs_node->state_history[history_index]);
				existing_predicted.push_back(predicted);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node_context;
				if (is_branch) {
					uniform_int_distribution<int> history_distribution(0, branch_node->branch_state_history.size()-1);
					int history_index = history_distribution(generator);
					double predicted = predict_helper(node_context,
													  is_branch,
													  branch_node->branch_state_history[history_index],
													  wrapper);
					existing_state.push_back(branch_node->branch_state_history[history_index]);
					existing_predicted.push_back(predicted);
				} else {
					uniform_int_distribution<int> history_distribution(0, branch_node->original_state_history.size()-1);
					int history_index = history_distribution(generator);
					double predicted = predict_helper(node_context,
													  is_branch,
													  branch_node->original_state_history[history_index],
													  wrapper);
					existing_state.push_back(branch_node->original_state_history[history_index]);
					existing_predicted.push_back(predicted);
				}
			}
			break;
		}
	}

	Network* existing_network = new Network(existing_state[0].size(),
											1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = train_distribution(generator);
		existing_network->activate(existing_state[index]);
		vector<double> errors{existing_predicted[index] - existing_network->output->acti_vals[0]};
		existing_network->backprop(errors);
	}

	vector<int> best_actions;
	AbstractNode* best_exit_next_node;
	double best_surprise = numeric_limits<double>::lowest();
	for (int e_index = 0; e_index < NUM_EXPLORE; e_index++) {
		vector<double> starting_state;
		switch (node_context->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node_context;
				uniform_int_distribution<int> history_distribution(0, obs_node->state_history.size()-1);
				int history_index = history_distribution(generator);
				starting_state = obs_node->state_history[history_index];
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node_context;
				if (is_branch) {
					uniform_int_distribution<int> history_distribution(0, branch_node->branch_state_history.size()-1);
					int history_index = history_distribution(generator);
					starting_state = branch_node->branch_state_history[history_index];
				} else {
					uniform_int_distribution<int> history_distribution(0, branch_node->original_state_history.size()-1);
					int history_index = history_distribution(generator);
					starting_state = branch_node->original_state_history[history_index];
				}
			}
			break;
		}
		existing_network->activate(starting_state);
		double existing_predicted = existing_network->output->acti_vals[0];

		AbstractNode* starting_node;
		switch (node_context->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node_context;
				starting_node = obs_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node_context;
				if (is_branch) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		}
		vector<AbstractNode*> possible_exits;
		wrapper->solution->random_exit_activate(
			starting_node,
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
		uniform_int_distribution<int> action_distribution(0, 3);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			curr_actions.push_back(action_distribution(generator));
		}

		double predicted = predict_helper(curr_actions,
										  curr_exit_next_node,
										  starting_state,
										  wrapper);

		double curr_surprise = predicted - existing_predicted;
		if (curr_surprise > best_surprise) {
			best_actions = curr_actions;
			best_exit_next_node = curr_exit_next_node;
			best_surprise = curr_surprise;
		}
	}

	vector<vector<double>> new_state;
	vector<double> new_predicted;
	for (int h_index = 0; h_index < TRAIN_NUM_SAMPLES; h_index++) {
		switch (node_context->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node_context;
				uniform_int_distribution<int> history_distribution(0, obs_node->state_history.size()-1);
				int history_index = history_distribution(generator);
				double predicted = predict_helper(best_actions,
												  best_exit_next_node,
												  obs_node->state_history[history_index],
												  wrapper);
				new_state.push_back(obs_node->state_history[history_index]);
				new_predicted.push_back(predicted);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node_context;
				if (is_branch) {
					uniform_int_distribution<int> history_distribution(0, branch_node->branch_state_history.size()-1);
					int history_index = history_distribution(generator);
					double predicted = predict_helper(best_actions,
													  best_exit_next_node,
													  branch_node->branch_state_history[history_index],
													  wrapper);
					new_state.push_back(branch_node->branch_state_history[history_index]);
					new_predicted.push_back(predicted);
				} else {
					uniform_int_distribution<int> history_distribution(0, branch_node->original_state_history.size()-1);
					int history_index = history_distribution(generator);
					double predicted = predict_helper(best_actions,
													  best_exit_next_node,
													  branch_node->original_state_history[history_index],
													  wrapper);
					new_state.push_back(branch_node->original_state_history[history_index]);
					new_predicted.push_back(predicted);
				}
			}
			break;
		}
	}

	Network* new_network = new Network(new_state[0].size(),
									   1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = train_distribution(generator);
		new_network->activate(new_state[index]);
		vector<double> errors{new_predicted[index] - new_network->output->acti_vals[0]};
		new_network->backprop(errors);
	}

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)new_state.size(); h_index++) {
		existing_network->activate(new_state[h_index]);
		new_network->activate(new_state[h_index]);
		if (new_network->output->acti_vals[0] > existing_network->output->acti_vals[0]) {
			sum_vals += new_predicted[h_index] - existing_network->output->acti_vals[0];
		}
	}
	double local_improvement = sum_vals / (double)new_state.size();

	double average_instances_per_run;
	switch (node_context->type) {
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)node_context;
			average_instances_per_run = obs_node->average_instances_per_run;
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
	double global_improvement = average_instances_per_run * local_improvement;

	bool is_success = false;
	if (local_improvement > 0.0) {
		if (wrapper->solution->train_new_last_scores.size() >= MIN_NUM_LAST_TRACK) {
			int num_better_than = 0;
			for (list<double>::iterator it = wrapper->solution->train_new_last_scores.begin();
					it != wrapper->solution->train_new_last_scores.end(); it++) {
				if (global_improvement >= *it) {
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
			wrapper->solution->train_new_last_scores.push_back(global_improvement);
		} else {
			wrapper->solution->train_new_last_scores.push_back(global_improvement);
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if (is_success || rand()%3 != 0) {
	#else
	if (is_success) {
	#endif /* MDEBUG */
		Experiment* experiment = new Experiment();

		experiment->node_context = node_context;
		experiment->is_branch = is_branch;

		experiment->actions = best_actions;
		experiment->exit_next_node = best_exit_next_node;

		experiment->original_network = existing_network;
		experiment->branch_network = new_network;

		switch (node_context->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node_context;
				obs_node->experiment = experiment;
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
	} else {
		delete existing_network;
		delete new_network;
	}
}
