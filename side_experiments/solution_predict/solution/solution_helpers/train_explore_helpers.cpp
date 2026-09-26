#include "solution_helpers.h"

#include <iostream>

#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_network.h"
#include "predict_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void train_explore_helper(SolutionWrapper* wrapper) {
	// temp
	wrapper->error_sum = 0.0;
	wrapper->error_count = 0;

	uniform_int_distribution<int> sample_distribution(0, wrapper->explore_scope_histories.size()-1);
	for (int iter_index = 0; iter_index < EXPLORE_ITERS_PER_BATCH; iter_index++) {
		int index = sample_distribution(generator);

		Eigen::VectorXf state;
		state.resize(NUM_STATES);
		state.setConstant(0.0);

		wrapper->solution->obs_network->activate(state,
												 wrapper->explore_starting_obs_histories[index]);

		bool is_done = false;
		TrainScopeHistory* train_scope_history = new TrainScopeHistory(wrapper->solution->starting_scope);
		wrapper->solution->starting_scope->train_activate(
			wrapper->explore_scope_histories[index],
			state,
			is_done,
			train_scope_history);

		wrapper->solution->score_network->activate(state);

		Eigen::VectorXf state_error;
		state_error.resize(NUM_STATES);
		state_error.setConstant(0.0);

		// temp
		wrapper->error_sum += abs(wrapper->explore_target_val_histories[index] - wrapper->solution->score_network->output->acti_vals(0));
		wrapper->error_count++;
		if (wrapper->solution->score_network->output->acti_vals(0) > wrapper->solution->score_network_max_val) {
			wrapper->solution->score_network->output->acti_vals(0) = wrapper->solution->score_network_max_val;
		}
		if (wrapper->solution->score_network->output->acti_vals(0) < wrapper->solution->score_network_min_val) {
			wrapper->solution->score_network->output->acti_vals(0) = wrapper->solution->score_network_min_val;
		}
		wrapper->solution->score_network->backprop(wrapper->explore_target_val_histories[index],
												   state_error);

		train_scope_history->backprop(wrapper->explore_target_val_histories[index],
									  state_error,
									  wrapper);

		wrapper->solution->obs_network->backprop(state_error);

		wrapper->solution->score_network->update();
		train_scope_history->update(wrapper->train_iter_index);
		wrapper->solution->obs_network->update();
		wrapper->train_iter_index++;

		delete train_scope_history;
	}

	// temp
	cout << "wrapper->error_sum: " << wrapper->error_sum << endl;
	cout << "wrapper->error_count: " << wrapper->error_count << endl;

	wrapper->explore_starting_obs_histories.clear();
	for (int h_index = 0; h_index < (int)wrapper->explore_scope_histories.size(); h_index++) {
		delete wrapper->explore_scope_histories[h_index];
	}
	wrapper->explore_scope_histories.clear();
	wrapper->explore_target_val_histories.clear();
}
