/**
 * - train signals on signals
 *   - but eval experiments on true
 */

#include "branch_end_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void BranchEndNode::update(double result,
						   BranchEndNodeHistory* history,
						   SolutionWrapper* wrapper) {
	history->signal_sum_vals += (result - wrapper->solution->curr_score);
	history->signal_sum_counts++;

	double average_val = history->signal_sum_vals / history->signal_sum_counts;

	if (this->pre_histories.size() >= MAX_NUM_DATAPOINTS) {
		uniform_int_distribution<int> distribution(0, this->pre_histories.size()-1);
		int index = distribution(generator);
		this->pre_histories[index] = history->pre_histories;
		this->post_histories[index] = history->post_histories;
		this->target_val_histories[index] = average_val;
	} else {
		this->pre_histories.push_back(history->pre_histories);
		this->post_histories.push_back(history->post_histories);
		this->target_val_histories.push_back(average_val);
	}
}

void BranchEndNode::backprop() {
	if (this->signal_network == NULL) {
		if (this->state_iter >= INITIAL_NUM_DATAPOINTS) {
			this->signal_network = new Network(this->pre_histories[0].size() + this->post_histories[0].size(),
											   NETWORK_SIZE_LARGE);

			uniform_int_distribution<int> distribution(0, this->pre_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = distribution(generator);

				vector<double> input = this->pre_histories[rand_index];
				input.insert(input.end(), this->post_histories[rand_index].begin(), this->post_histories[rand_index].end());

				this->signal_network->activate(input);

				double error = this->target_val_histories[rand_index] - this->signal_network->output->acti_vals[0];

				this->signal_network->backprop(error);
			}

			this->state_iter = 0;
		}
	} else {
		if (this->state_iter >= UPDATE_NUM_DATAPOINTS) {
			uniform_int_distribution<int> distribution(0, this->pre_histories.size()-1);
			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				int rand_index = distribution(generator);

				vector<double> input = this->pre_histories[rand_index];
				input.insert(input.end(), this->post_histories[rand_index].begin(), this->post_histories[rand_index].end());

				this->signal_network->activate(input);

				double error = this->target_val_histories[rand_index] - this->signal_network->output->acti_vals[0];

				this->signal_network->backprop(error);
			}

			this->state_iter = 0;
		}
	}
}
