#include "branch_end_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "network.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "solution_wrapper.h"

// temp
#include "minesweeper.h"

using namespace std;

void BranchEndNode::experiment_step(vector<double>& obs,
									int& action,
									bool& is_next,
									SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchEndNodeHistory* history = new BranchEndNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	bool is_existing = false;
	if (wrapper->curr_experiment != NULL) {
		switch (wrapper->curr_experiment->type) {
		case EXPERIMENT_TYPE_BRANCH:
			{
				BranchExperiment* branch_experiment = (BranchExperiment*)wrapper->curr_experiment;
				switch (branch_experiment->state) {
				case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
					is_existing = true;
					break;
				}
			}
			break;
		case EXPERIMENT_TYPE_PASS_THROUGH:
			{
				PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)wrapper->curr_experiment;
				switch (pass_through_experiment->state) {
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
					is_existing = true;
					break;
				}
			}
			break;
		}
	}

	bool need_callback = false;
	for (int c_index = 0; c_index < (int)wrapper->experiment_callbacks.size(); c_index++) {
		if (wrapper->experiment_callbacks[c_index].size() > 0
				&& wrapper->experiment_callbacks[c_index].back() == this->branch_start_node) {
			need_callback = true;
			break;
		}
	}
	for (int c_index = 0; c_index < (int)wrapper->branch_end_node_callbacks.size(); c_index++) {
		if (wrapper->branch_end_node_callbacks[c_index].size() > 0
				&& wrapper->branch_end_node_callbacks[c_index].back() == this->branch_start_node) {
			need_callback = true;
			break;
		}
	}

	if (need_callback && this->pre_network != NULL) {
		this->pre_network->activate(wrapper->branch_node_stack_obs.back());

		vector<double> input = wrapper->branch_node_stack_obs.back();
		input.insert(input.end(), obs.begin(), obs.end());
		this->post_network->activate(input);

		double signal = this->post_network->output->acti_vals[0] - this->pre_network->output->acti_vals[0];

		// for (int c_index = 0; c_index < (int)wrapper->experiment_callbacks.size(); c_index++) {
		// 	if (wrapper->experiment_callbacks[c_index].size() > 0
		// 			&& wrapper->experiment_callbacks[c_index].back() == this->branch_start_node) {
		// 		wrapper->experiment_history->signal_sum_vals[c_index] += signal;
		// 		wrapper->experiment_history->signal_sum_counts[c_index]++;

		// 		wrapper->experiment_callbacks[c_index].pop_back();
		// 	}
		// }
		for (int c_index = 0; c_index < (int)wrapper->branch_end_node_callbacks.size(); c_index++) {
			if (wrapper->branch_end_node_callbacks[c_index].size() > 0
					&& wrapper->branch_end_node_callbacks[c_index].back() == this->branch_start_node) {
				wrapper->branch_end_node_callback_histories[c_index]->signal_sum_vals += signal;
				wrapper->branch_end_node_callback_histories[c_index]->signal_sum_counts++;

				wrapper->branch_end_node_callbacks[c_index].pop_back();
			}
		}

		// // temp
		// bool is_explore = false;
		// if (wrapper->curr_experiment != NULL) {
		// 	switch (wrapper->curr_experiment->type) {
		// 	case EXPERIMENT_TYPE_BRANCH:
		// 		{
		// 			BranchExperiment* branch_experiment = (BranchExperiment*)wrapper->curr_experiment;
		// 			switch (branch_experiment->state) {
		// 			case BRANCH_EXPERIMENT_STATE_EXPLORE:
		// 			case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		// 				is_explore = true;
		// 				break;
		// 			}
		// 		}
		// 		break;
		// 	case EXPERIMENT_TYPE_PASS_THROUGH:
		// 		{
		// 			PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)wrapper->curr_experiment;
		// 			switch (pass_through_experiment->state) {
		// 			case PASS_THROUGH_EXPERIMENT_STATE_C1:
		// 			case PASS_THROUGH_EXPERIMENT_STATE_C2:
		// 				is_explore = true;
		// 				break;
		// 			}
		// 		}
		// 		break;
		// 	}
		// }
		// if (is_explore) {
		// 	cout << "this->parent->id: " << this->parent->id << endl;
		// 	cout << "this->id: " << this->id << endl;
		// 	cout << "pre_obs:" << endl;
		// 	for (int i_index = 0; i_index < 5; i_index++) {
		// 		for (int j_index = 0; j_index < 5; j_index++) {
		// 			cout << wrapper->branch_node_stack_obs.back()[5 * i_index + j_index] << " ";
		// 		}
		// 		cout << endl;
		// 	}
		// 	cout << "post_obs:" << endl;
		// 	for (int i_index = 0; i_index < 5; i_index++) {
		// 		for (int j_index = 0; j_index < 5; j_index++) {
		// 			cout << obs[5 * i_index + j_index] << " ";
		// 		}
		// 		cout << endl;
		// 	}

		// 	cout << "this->pre_network->output->acti_vals[0]: " << this->pre_network->output->acti_vals[0] << endl;

		// 	cout << "this->post_network->output->acti_vals[0]: " << this->post_network->output->acti_vals[0] << endl;

		// 	cout << endl;
		// }
	}

	if (is_existing || need_callback) {
		history->pre_histories = wrapper->branch_node_stack_obs.back();
		history->post_histories = obs;

		// temp
		history->start_location = wrapper->branch_node_location.back();
		Minesweeper* minesweeper = (Minesweeper*)wrapper->problem;
		history->end_location = {minesweeper->current_x, minesweeper->current_y};
	}

	wrapper->branch_node_stack.pop_back();
	wrapper->branch_node_stack_obs.pop_back();
	// temp
	wrapper->branch_node_location.pop_back();

	if (is_existing || need_callback) {
		wrapper->branch_end_node_callbacks.push_back(wrapper->branch_node_stack);
		wrapper->branch_end_node_callback_histories.push_back(history);
	}

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			false,
			wrapper);
	}
}
