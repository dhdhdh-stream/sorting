/**
 * - training predict on existing even slightly greatly pushes away from predicting
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

void train_predict_helper(SolutionWrapper* wrapper,
						  double target_val) {
	for (int i_index = 0; i_index < (int)wrapper->predict_scope_histories.size(); i_index++) {
		ScopeHistory* scope_history = wrapper->predict_scope_histories[i_index];
		Scope* scope = scope_history->scope;

		Eigen::VectorXf state;
		state.resize(NUM_STATES);
		state.setConstant(0.0);

		scope->obs_network->activate(state,
									 scope_history->obs);

		vector<TrainAbstractNodeHistory*> train_node_histories;
		int explore_index = wrapper->predict_indexes[i_index];
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

		scope->score_network->backprop(target_val,
									   state_error);

		for (int h_index = (int)train_node_histories.size()-1; h_index >= 0; h_index--) {
			train_node_histories[h_index]->backprop(state_error);
		}

		scope->obs_network->backprop(state_error);

		for (int h_index = 0; h_index < (int)train_node_histories.size(); h_index++) {
			delete train_node_histories[h_index];
		}
	}

	for (int i_index = 0; i_index < (int)wrapper->predict_scope_histories.size(); i_index++) {
		ScopeHistory* scope_history = wrapper->predict_scope_histories[i_index];
		Scope* scope = scope_history->scope;

		if (scope->score_network->last_update_iter != wrapper->train_iter_index) {
			scope->score_network->update();

			scope->score_network->last_update_iter = wrapper->train_iter_index;
		}

		int explore_index = wrapper->predict_indexes[i_index];
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
	}
	wrapper->train_iter_index++;

	wrapper->predict_scope_histories.clear();
	wrapper->predict_indexes.clear();
}

void train_all_predict_helper(ScopeHistory* scope_history,
							  double target_val) {
	Scope* scope = scope_history->scope;

	Eigen::VectorXf state;
	state.resize(NUM_STATES);
	state.setConstant(0.0);

	scope->obs_network->activate(state,
								 scope_history->obs);

	vector<TrainAbstractNodeHistory*> train_node_histories;
	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNode* node = scope_history->node_histories[h_index]->node;
		node->train_step(scope_history->node_histories[h_index],
						 state,
						 train_node_histories);
	}

	scope->score_network->activate(state);

	Eigen::VectorXf state_error;
	state_error.resize(NUM_STATES);
	state_error.setConstant(0.0);

	scope->score_network->backprop(target_val,
								   state_error);

	for (int h_index = (int)train_node_histories.size()-1; h_index >= 0; h_index--) {
		train_node_histories[h_index]->backprop(state_error);
	}

	scope->obs_network->backprop(state_error);

	for (int h_index = 0; h_index < (int)train_node_histories.size(); h_index++) {
		delete train_node_histories[h_index];
	}

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNode* node = scope_history->node_histories[h_index]->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[h_index];
				train_all_predict_helper(scope_node_history->scope_history,
										 target_val);
			}
			break;
		}
	}
}
