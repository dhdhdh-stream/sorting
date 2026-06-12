/**
 * - need long-trained network for end network
 *   - need long-trained to predict crazy correctly
 * 
 * TODO: instead of predicting until end, predict until signal end of scope
 */

#include "experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment_run.h"
#include "globals.h"
#include "network.h"
#include "run.h"
#include "solution.h"
#include "solution_helpers.h"
#include "state_network.h"
#include "start_node.h"
#include "world_model.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int GATHER_NUM_SAMPLES = 10;
#else
const int GATHER_NUM_SAMPLES = 100;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_SIMULATE = 40;
#else
const int NUM_SIMULATE = 4000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_EXPLORE = 5;
#else
const int NUM_EXPLORE = 1000;
#endif /* MDEBUG */

void Experiment::gather_activate(ExperimentRun* run) {
	map<Experiment*, ExperimentHistory*>::iterator it =
		run->experiment_histories.find(this);
	if (it == run->experiment_histories.end()) {
		run->experiment_histories[this] = new ExperimentHistory(this);
	}

	this->start_state_history.push_back(run->state);
}

void Experiment::gather_backprop(double target_val,
								 ExperimentHistory* history,
								 Wrapper* wrapper) {
	this->state_iter++;
	if (this->state_iter >= GATHER_NUM_SAMPLES) {
		uniform_int_distribution<int> start_distribution(0, this->start_state_history.size()-1);
		uniform_int_distribution<int> train_distribution(0, NUM_SIMULATE-1);

		AbstractNode* next_node;
		switch (this->node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->node_context;
				next_node = start_node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				next_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
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
			train_existing_state[iter_index] = this->start_state_history[start_index];
			train_existing_predicted[iter_index] = predict_helper(this->start_state_history[start_index],
																  next_node,
																  wrapper);
		}

		this->original_network = new Network(this->start_state_history[0].size());
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = train_distribution(generator);
			this->original_network->activate(train_existing_state[index]);
			double error = train_existing_predicted[index] - this->original_network->output->acti_vals[0];
			this->original_network->backprop(error);
		}

		// temp
		for (int h_index = 0; h_index < 10; h_index++) {
			cout << h_index << endl;
			this->original_network->activate(train_existing_state[h_index]);
			cout << "this->original_network->output->acti_vals[0]: " << this->original_network->output->acti_vals[0] << endl;
			cout << "train_existing_predicted[h_index]: " << train_existing_predicted[h_index] << endl;
		}

		vector<int> best_actions;
		AbstractNode* best_exit_next_node;
		double best_surprise = numeric_limits<double>::lowest();
		for (int e_index = 0; e_index < NUM_EXPLORE; e_index++) {
			int start_index = start_distribution(generator);

			// // temp
			// if (e_index < 10) {
			// 	cout << e_index << endl;
			// }

			vector<double> state = this->start_state_history[start_index];
			// // temp
			// if (e_index < 10) {
			// 	cout << "start state:";
			// 	for (int s_index = 0; s_index < (int)state.size(); s_index++) {
			// 		cout << " " << state[s_index];
			// 	}
			// 	cout << endl;
			// }

			this->original_network->activate(state);
			double existing_predicted = this->original_network->output->acti_vals[0];
			// // temp
			// if (e_index < 10) {
			// 	cout << "existing_predicted: " << existing_predicted << endl;
			// }

			vector<AbstractNode*> possible_exits;
			wrapper->solution->random_exit_activate(
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
			uniform_int_distribution<int> action_distribution(0, 3);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				curr_actions.push_back(action_distribution(generator));
			}
			// // temp
			// if (e_index < 10) {
			// 	cout << "curr_actions:";
			// 	for (int a_index = 0; a_index < (int)curr_actions.size(); a_index++) {
			// 		cout << " " << curr_actions[a_index];
			// 	}
			// 	cout << endl;
			// }

			for (int a_index = 0; a_index < (int)curr_actions.size(); a_index++) {
				action_helper(curr_actions[a_index],
							  state,
							  wrapper);

				predict_helper(state,
							   wrapper);
			}
			double predicted = predict_helper(state,
											  curr_exit_next_node,
											  wrapper);

			double curr_surprise = predicted - existing_predicted;
			if (curr_surprise > best_surprise) {
				best_actions = curr_actions;
				best_exit_next_node = curr_exit_next_node;
				best_surprise = curr_surprise;
			}
		}

		// // temp
		// cout << "this->node_context->id: " << this->node_context->id << endl;
		// cout << "best_exit_next_node->id: " << best_exit_next_node->id << endl;
		// cout << "best_actions:";
		// for (int a_index = 0; a_index < (int)best_actions.size(); a_index++) {
		// 	cout << " " << best_actions[a_index];
		// }
		// cout << endl;
		// cout << "best_surprise: " << best_surprise << endl;

		vector<vector<double>> train_new_state(NUM_SIMULATE);
		vector<double> train_new_predicted(NUM_SIMULATE);
		for (int iter_index = 0; iter_index < NUM_SIMULATE; iter_index++) {
			int start_index = start_distribution(generator);

			train_new_state[iter_index] = this->start_state_history[start_index];

			vector<double> state = this->start_state_history[start_index];
			for (int a_index = 0; a_index < (int)best_actions.size(); a_index++) {
				action_helper(best_actions[a_index],
							  state,
							  wrapper);

				predict_helper(state,
							   wrapper);
			}
			train_new_predicted[iter_index] = predict_helper(state,
															 best_exit_next_node,
															 wrapper);
		}

		// // temp
		// for (int h_index = 0; h_index < 10; h_index++) {
		// 	cout << h_index << endl;
		// 	cout << "train_new_predicted[h_index]: " << train_new_predicted[h_index] << endl;
		// }

		this->branch_network = new Network(this->start_state_history[0].size());

		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = train_distribution(generator);
			this->branch_network->activate(train_new_state[index]);
			double error = train_new_predicted[index] - this->branch_network->output->acti_vals[0];
			this->branch_network->backprop(error);
		}

		// // temp
		// for (int h_index = 0; h_index < 10; h_index++) {
		// 	cout << h_index << endl;
		// 	this->branch_network->activate(this->start_state_history[h_index]);
		// 	cout << "this->branch_network->output->acti_vals[0]: " << this->branch_network->output->acti_vals[0] << endl;
		// 	cout << "train_new_predicted[h_index]: " << train_new_predicted[h_index] << endl;
		// }

		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)train_new_state.size(); h_index++) {
			this->original_network->activate(train_new_state[h_index]);
			this->branch_network->activate(train_new_state[h_index]);
			if (this->branch_network->output->acti_vals[0] > this->original_network->output->acti_vals[0]) {
				sum_vals += train_new_predicted[h_index] - this->original_network->output->acti_vals[0];
			}

			// // temp
			// if (h_index < 10) {
			// 	cout << "this->original_network->output->acti_vals[0]: " << this->original_network->output->acti_vals[0] << endl;
			// 	cout << "this->branch_network->output->acti_vals[0]: " << this->branch_network->output->acti_vals[0] << endl;
			// 	cout << "new_predicted[h_index]: " << new_predicted[h_index] << endl;
			// }
		}
		this->predicted_local_improvement = sum_vals / (double)train_new_state.size();

		int total_iters = wrapper->iter - this->starting_iter;
		if (total_iters < 0) {
			total_iters += numeric_limits<int>::max();
		}
		double average_instances_per_run = (double)this->start_state_history.size() / (double)total_iters;

		this->predicted_global_improvement = average_instances_per_run * this->predicted_local_improvement;

		// temp
		cout << "this->predicted_local_improvement: " << this->predicted_local_improvement << endl;
		cout << "this->predicted_global_improvement: " << this->predicted_global_improvement << endl;

		bool is_success = false;
		if (this->predicted_local_improvement > 0.0) {
			if (wrapper->solution->train_new_last_scores.size() >= MIN_NUM_LAST_TRACK) {
				int num_better_than = 0;
				for (list<double>::iterator it = wrapper->solution->train_new_last_scores.begin();
						it != wrapper->solution->train_new_last_scores.end(); it++) {
					if (this->predicted_global_improvement >= *it) {
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
				wrapper->solution->train_new_last_scores.push_back(this->predicted_global_improvement);
			} else {
				wrapper->solution->train_new_last_scores.push_back(this->predicted_global_improvement);
			}

			// temp
			is_success = true;
		}

		if (is_success) {
			this->actions = best_actions;
			this->exit_next_node = best_exit_next_node;

			this->start_state_history.clear();

			this->curr_ramp = 0;
			this->measure_status = MEASURE_STATUS_N_A;

			this->total_count = 0;
			this->existing_sum_scores = 0.0;
			this->existing_count = 0;
			this->new_sum_scores = 0.0;
			this->new_count = 0;

			this->state = EXPERIMENT_STATE_RAMP;
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
			delete this;
		}
	}
}
