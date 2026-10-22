/**
 * - using predict for update and explore can be great
 *   - ...but only if predict's not so bad that it greatly damages existing
 * 
 * - training on both existing and explore pushes away from predicting
 *   - training on existing increases existing score
 *   - training on explore reduces predict score
 */

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

void train_predict_helper(Scope* scope,
						  SolutionWrapper* wrapper) {
	uniform_int_distribution<int> sample_distribution(0, scope->predict_scope_histories.size()-1);
	for (int iter_index = 0; iter_index < ITERS_PER_BATCH; iter_index++) {
		int index = sample_distribution(generator);

		ScopeHistory* scope_history = scope->predict_scope_histories[index];

		Eigen::VectorXf state;
		state.resize(NUM_STATES);
		state.setConstant(0.0);

		scope->obs_network->activate(state,
									 scope_history->obs);

		int explore_index = scope->predict_index_histories[index];

		vector<TrainAbstractNodeHistory*> pre_train_node_histories;
		for (int h_index = 0; h_index <= explore_index; h_index++) {
			AbstractNode* node = scope_history->node_histories[h_index]->node;
			node->train_step(scope_history->node_histories[h_index],
							 state,
							 pre_train_node_histories);
		}

		vector<TrainAbstractNodeHistory*> post_train_node_histories;
		for (int h_index = explore_index+1; h_index < (int)scope_history->node_histories.size(); h_index++) {
			AbstractNode* node = scope_history->node_histories[h_index]->node;
			node->train_predict_step(scope_history->node_histories[h_index],
									 state,
									 post_train_node_histories);
		}

		scope->score_network->activate(state);

		Eigen::VectorXf state_error;
		state_error.resize(NUM_STATES);
		state_error.setConstant(0.0);

		scope->score_network->backprop(scope->predict_target_val_histories[index],
									   state_error);

		for (int h_index = (int)post_train_node_histories.size()-1; h_index >= 0; h_index--) {
			post_train_node_histories[h_index]->backprop(state_error);
		}

		for (int h_index = (int)pre_train_node_histories.size()-1; h_index >= 0; h_index--) {
			pre_train_node_histories[h_index]->backprop(state_error);
		}

		scope->obs_network->backprop(state_error);

		for (int h_index = 0; h_index < (int)pre_train_node_histories.size(); h_index++) {
			delete pre_train_node_histories[h_index];
		}
		for (int h_index = 0; h_index < (int)post_train_node_histories.size(); h_index++) {
			delete post_train_node_histories[h_index];
		}

		if (scope->score_network->last_update_iter != wrapper->train_iter_index) {
			scope->score_network->update();

			scope->score_network->last_update_iter = wrapper->train_iter_index;
		}

		for (int h_index = 0; h_index <= explore_index; h_index++) {
			AbstractNode* node = scope_history->node_histories[h_index]->node;
			node->update_step(wrapper->train_iter_index);
		}
		for (int h_index = explore_index+1; h_index < (int)scope_history->node_histories.size(); h_index++) {
			AbstractNode* node = scope_history->node_histories[h_index]->node;
			node->update_predict_step(wrapper->train_iter_index);
		}

		if (scope->obs_network->last_update_iter != wrapper->train_iter_index) {
			scope->obs_network->update();

			scope->obs_network->last_update_iter = wrapper->train_iter_index;
		}

		wrapper->train_iter_index++;
	}

	scope->predict_scope_histories.clear();
	scope->predict_index_histories.clear();
	scope->predict_target_val_histories.clear();
}
