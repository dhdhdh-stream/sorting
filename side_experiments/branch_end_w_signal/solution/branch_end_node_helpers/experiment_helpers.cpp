#include "branch_end_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void BranchEndNode::experiment_step(vector<double>& obs,
									int& action,
									bool& is_next,
									SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchEndNodeHistory* history = new BranchEndNodeHistory(this);
	scope_history->node_histories[this->id] = history;

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

	if (need_callback) {
		if (this->pre_network != NULL) {
			this->pre_network->activate(wrapper->branch_node_stack_obs.back());

			vector<double> input = wrapper->branch_node_stack_obs.back();
			input.insert(input.end(), obs.begin(), obs.end());
			this->post_network->activate(input);

			double signal = this->post_network->output->acti_vals[0] - this->pre_network->output->acti_vals[0];

			for (int c_index = 0; c_index < (int)wrapper->experiment_callbacks.size(); c_index++) {
				if (wrapper->experiment_callbacks[c_index].size() > 0
						&& wrapper->experiment_callbacks[c_index].back() == this->branch_start_node) {
					wrapper->experiment_history->signal_sum_vals[c_index] += signal;
					wrapper->experiment_history->signal_sum_counts[c_index]++;

					wrapper->experiment_callbacks[c_index].pop_back();
				}
			}
			for (int c_index = 0; c_index < (int)wrapper->branch_end_node_callbacks.size(); c_index++) {
				if (wrapper->branch_end_node_callbacks[c_index].size() > 0
						&& wrapper->branch_end_node_callbacks[c_index].back() == this->branch_start_node) {
					wrapper->branch_end_node_callback_histories[c_index]->signal_sum_vals += signal;
					wrapper->branch_end_node_callback_histories[c_index]->signal_sum_counts++;

					wrapper->branch_end_node_callbacks[c_index].pop_back();
				}
			}
		}

		switch (wrapper->curr_experiment->type) {
		case EXPERIMENT_TYPE_BRANCH:
			{
				BranchExperiment* branch_experiment = (BranchExperiment*)wrapper->curr_experiment;
				switch (branch_experiment->state) {
				case BRANCH_EXPERIMENT_STATE_EXPLORE:
				case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
					history->pre_histories = wrapper->branch_node_stack_obs.back();
					history->post_histories = obs;
					break;
				}
			}
			break;
		}
	}

	wrapper->branch_node_stack.pop_back();
	wrapper->branch_node_stack_obs.pop_back();

	if (need_callback) {
		switch (wrapper->curr_experiment->type) {
		case EXPERIMENT_TYPE_BRANCH:
			{
				BranchExperiment* branch_experiment = (BranchExperiment*)wrapper->curr_experiment;
				switch (branch_experiment->state) {
				case BRANCH_EXPERIMENT_STATE_EXPLORE:
				case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
					wrapper->branch_end_node_callbacks.push_back(wrapper->branch_node_stack);
					wrapper->branch_end_node_callback_histories.push_back(history);
					break;
				}
			}
			break;
		}
	}

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			false,
			wrapper);
	}
}
