/**
 * - train signals on signals
 *   - but eval experiments on true
 */

#include "branch_end_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "pass_through_experiment.h"
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

	bool is_existing = false;
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
	if (is_existing) {
		if (this->existing_pre_histories.size() >= MAX_NUM_DATAPOINTS) {
			uniform_int_distribution<int> distribution(0, this->existing_pre_histories.size()-1);
			int index = distribution(generator);
			this->existing_pre_histories[index] = history->pre_histories;
			this->existing_post_histories[index] = history->post_histories;
			this->existing_target_val_histories[index] = average_val;
			// temp
			this->start_locations[index] = history->start_location;
			this->end_locations[index] = history->end_location;
		} else {
			this->existing_pre_histories.push_back(history->pre_histories);
			this->existing_post_histories.push_back(history->post_histories);
			this->existing_target_val_histories.push_back(average_val);
			// temp
			this->start_locations.push_back(history->start_location);
			this->end_locations.push_back(history->end_location);
		}
	} else {
		if (this->explore_pre_histories.size() >= MAX_NUM_DATAPOINTS) {
			uniform_int_distribution<int> distribution(0, this->explore_pre_histories.size()-1);
			int index = distribution(generator);
			this->explore_pre_histories[index] = history->pre_histories;
			this->explore_post_histories[index] = history->post_histories;
			this->explore_target_val_histories[index] = average_val;
		} else {
			this->explore_pre_histories.push_back(history->pre_histories);
			this->explore_post_histories.push_back(history->post_histories);
			this->explore_target_val_histories.push_back(average_val);
		}
	}
}

void BranchEndNode::backprop() {
	if (this->existing_pre_histories.size() >= MIN_NUM_DATAPOINTS
			&& this->explore_pre_histories.size() >= MIN_NUM_DATAPOINTS) {
		if (this->pre_network == NULL) {
			this->pre_network = new Network(this->existing_pre_histories[0].size(),
											NETWORK_SIZE_SMALL);
			this->post_network = new Network(this->existing_pre_histories[0].size() + this->existing_post_histories[0].size(),
											 NETWORK_SIZE_LARGE);

			uniform_int_distribution<int> is_existing_distribution(0, 1);
			uniform_int_distribution<int> existing_distribution(0, this->existing_pre_histories.size()-1);
			uniform_int_distribution<int> explore_distribution(0, this->explore_pre_histories.size()-1);

			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					int rand_index = existing_distribution(generator);

					this->pre_network->activate(this->existing_pre_histories[rand_index]);

					double error = this->existing_target_val_histories[rand_index] - this->pre_network->output->acti_vals[0];

					this->pre_network->backprop(error);
				} else {
					int rand_index = explore_distribution(generator);

					this->pre_network->activate(this->explore_pre_histories[rand_index]);

					double error = this->explore_target_val_histories[rand_index] - this->pre_network->output->acti_vals[0];

					this->pre_network->backprop(error);
				}
			}

			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					int rand_index = existing_distribution(generator);

					vector<double> input = this->existing_pre_histories[rand_index];
					input.insert(input.end(), this->existing_post_histories[rand_index].begin(), this->existing_post_histories[rand_index].end());

					this->post_network->activate(input);

					double error = this->existing_target_val_histories[rand_index] - this->post_network->output->acti_vals[0];

					this->post_network->backprop(error);
				} else {
					int rand_index = explore_distribution(generator);

					vector<double> input = this->explore_pre_histories[rand_index];
					input.insert(input.end(), this->explore_post_histories[rand_index].begin(), this->explore_post_histories[rand_index].end());

					this->post_network->activate(input);

					double error = this->explore_target_val_histories[rand_index] - this->post_network->output->acti_vals[0];

					this->post_network->backprop(error);
				}
			}
		} else {
			uniform_int_distribution<int> is_existing_distribution(0, 1);
			uniform_int_distribution<int> existing_distribution(0, this->existing_pre_histories.size()-1);
			uniform_int_distribution<int> explore_distribution(0, this->explore_pre_histories.size()-1);

			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					int rand_index = existing_distribution(generator);

					this->pre_network->activate(this->existing_pre_histories[rand_index]);

					double error = this->existing_target_val_histories[rand_index] - this->pre_network->output->acti_vals[0];

					this->pre_network->backprop(error);
				} else {
					int rand_index = explore_distribution(generator);

					this->pre_network->activate(this->explore_pre_histories[rand_index]);

					double error = this->explore_target_val_histories[rand_index] - this->pre_network->output->acti_vals[0];

					this->pre_network->backprop(error);
				}
			}

			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					int rand_index = existing_distribution(generator);

					vector<double> input = this->existing_pre_histories[rand_index];
					input.insert(input.end(), this->existing_post_histories[rand_index].begin(), this->existing_post_histories[rand_index].end());

					this->post_network->activate(input);

					double error = this->existing_target_val_histories[rand_index] - this->post_network->output->acti_vals[0];

					this->post_network->backprop(error);
				} else {
					int rand_index = explore_distribution(generator);

					vector<double> input = this->explore_pre_histories[rand_index];
					input.insert(input.end(), this->explore_post_histories[rand_index].begin(), this->explore_post_histories[rand_index].end());

					this->post_network->activate(input);

					double error = this->explore_target_val_histories[rand_index] - this->post_network->output->acti_vals[0];

					this->post_network->backprop(error);
				}
			}
		}

		// temp
		map<pair<int,int>, int> travel_tracker;
		for (int h_index = 0; h_index < (int)this->start_locations.size(); h_index++) {
			pair<int, int> travel = {
				this->end_locations[h_index].first - this->start_locations[h_index].first,
				this->end_locations[h_index].second - this->start_locations[h_index].second,
			};

			map<pair<int,int>, int>::iterator it = travel_tracker.find(travel);
			if (it == travel_tracker.end()) {
				travel_tracker[travel] = 1;
			} else {
				it->second++;
			}
		}
		this->travel_counts.clear();
		for (map<pair<int,int>, int>::iterator it = travel_tracker.begin();
				it != travel_tracker.end(); it++) {
			this->travel_counts.push_back(it->second);
		}
	}
}
