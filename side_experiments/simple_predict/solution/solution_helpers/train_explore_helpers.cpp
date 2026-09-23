#include "solution_helpers.h"

#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "predict_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void train_explore_helper(SolutionWrapper* wrapper) {
	uniform_int_distribution<int> sample_distribution(0, wrapper->explore_scope_histories.size()-1);
	for (int iter_index = 0; iter_index < ITERS_PER_BATCH; iter_index++) {
		int index = sample_distribution(generator);

		Eigen::VectorXf state;
		state.resize(NUM_STATES);
		state.setConstant(0.0);
		bool is_done = false;
		TrainScopeHistory* train_scope_history = new TrainScopeHistory(wrapper->solution->starting_scope);
		wrapper->solution->starting_scope->train_activate(
			wrapper->existing_scope_histories[index],
			state,
			is_done,
			train_scope_history);

		Eigen::VectorXf state_error;
		state_error.resize(NUM_STATES);
		state_error.setConstant(0.0);
		train_scope_history->backprop(wrapper->existing_target_val_histories[index],
									  state_error);

		delete train_scope_history;
	}

	for (int h_index = 0; h_index < (int)wrapper->explore_scope_histories.size(); h_index++) {
		delete wrapper->explore_scope_histories[h_index];
	}
	wrapper->explore_scope_histories.clear();
	wrapper->explore_target_val_histories.clear();
}
