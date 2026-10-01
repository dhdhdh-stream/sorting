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
	for (int iter_index = 0; iter_index < ITERS_PER_BATCH; iter_index++) {
		int index = sample_distribution(generator);

		ScopeHistory* scope_history = wrapper->explore_scope_histories[index];
		Scope* scope = scope_history->scope;

		Eigen::VectorXf state;
		state.resize(NUM_STATES);
		state.setConstant(0.0);

		scope->obs_network->activate(state,
									 scope_history->obs);

		vector<TrainAbstractNodeHistory*> train_node_histories;
		int explore_index = wrapper->explore_index_histories[index];
		for (int h_index = 0; h_index <= explore_index; h_index++) {
			AbstractNode* node = scope_history->node_histories[h_index]->node;
			node->train_step(scope_history->node_histories[h_index],
							 state,
							 train_node_histories);
		}
		for (int h_index = explore_index+1; h_index < (int)scope_history->node_histories.size(); h_index++) {
			AbstractNode* node = scope_history->node_histories[h_index]->node;
			node->train_predict_step(scope_history->node_histories[h_index],
									 state,
									 train_node_histories);
		}

		scope->score_network->activate(state);

		Eigen::VectorXf state_error;
		state_error.resize(NUM_STATES);
		state_error.setConstant(0.0);

		// temp
		double target_val = wrapper->explore_target_val_histories[index];
		wrapper->error_sum += abs(target_val - scope->score_network->output->acti_vals(0));
		wrapper->error_count++;
		scope->score_network->backprop(target_val,
									   state_error);

		for (int h_index = (int)train_node_histories.size()-1; h_index >= 0; h_index--) {
			train_node_histories[h_index]->backprop(state_error);
		}

		scope->obs_network->backprop(state_error);

		scope->score_network->update();

		for (int h_index = (int)train_node_histories.size()-1; h_index >= 0; h_index--) {
			train_node_histories[h_index]->update(wrapper->train_iter_index);
		}
		wrapper->train_iter_index++;

		scope->obs_network->update();

		for (int h_index = 0; h_index < (int)train_node_histories.size(); h_index++) {
			delete train_node_histories[h_index];
		}
	}

	// temp
	cout << "wrapper->error_sum: " << wrapper->error_sum << endl;
	cout << "wrapper->error_count: " << wrapper->error_count << endl;

	for (int h_index = 0; h_index < (int)wrapper->explore_scope_histories.size(); h_index++) {
		delete wrapper->explore_scope_histories[h_index];
	}
	wrapper->explore_scope_histories.clear();
	wrapper->explore_index_histories.clear();
	wrapper->explore_target_val_histories.clear();
}
