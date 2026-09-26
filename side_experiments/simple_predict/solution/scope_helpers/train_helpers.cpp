#include "scope.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_network.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void Scope::train_activate(ScopeHistory* history,
						   Eigen::VectorXf& state,
						   bool& is_done,
						   TrainScopeHistory* train_scope_history) {
	this->obs_network->activate(state,
								history->obs);
	train_scope_history->obs_network_history = new ObsNetworkHistory();
	this->obs_network->save(train_scope_history->obs_network_history);

	if (history->explore_index == -1) {
		for (int h_index = 0; h_index < (int)history->node_histories.size(); h_index++) {
			AbstractNode* node = history->node_histories[h_index]->node;
			node->train_step(history->node_histories[h_index],
							 state,
							 is_done,
							 train_scope_history);

			if (is_done) {
				break;
			}
		}

		train_scope_history->is_explore = false;
	} else {
		for (int h_index = 0; h_index <= history->explore_index; h_index++) {
			AbstractNode* node = history->node_histories[h_index]->node;
			node->train_step(history->node_histories[h_index],
							 state,
							 is_done,
							 train_scope_history);
		}
		for (int h_index = history->explore_index+1; h_index < (int)history->node_histories.size(); h_index++) {
			AbstractNode* node = history->node_histories[h_index]->node;
			node->train_predict_step(history->node_histories[h_index],
									 state,
									 train_scope_history);
		}

		train_scope_history->is_explore = true;
		this->score_network->activate(state);
		train_scope_history->score_network_history = new ScoreNetworkHistory();
		this->score_network->save(train_scope_history->score_network_history);

		is_done = true;
	}
}

void TrainScopeHistory::backprop(double target_val,
								 Eigen::VectorXf& state_error,
								 SolutionWrapper* wrapper) {
	if (this->is_explore) {
		this->scope->score_network->load(this->score_network_history);
		// temp
		wrapper->error_sum += abs(target_val - this->scope->score_network->output->acti_vals(0));
		wrapper->error_count++;
		if (this->scope->score_network->output->acti_vals(0) > wrapper->solution->score_network_max_val) {
			this->scope->score_network->output->acti_vals(0) = wrapper->solution->score_network_max_val;
		}
		if (this->scope->score_network->output->acti_vals(0) < wrapper->solution->score_network_min_val) {
			this->scope->score_network->output->acti_vals(0) = wrapper->solution->score_network_min_val;
		}
		this->scope->score_network->backprop(target_val,
											 state_error);
	}

	for (int h_index = (int)this->node_histories.size()-1; h_index >= 0; h_index--) {
		this->node_histories[h_index]->backprop(target_val,
												state_error,
												wrapper);
	}

	this->scope->obs_network->load(this->obs_network_history);
	this->scope->obs_network->backprop(state_error);
}

void TrainScopeHistory::update(int iter_index) {
	if (this->is_explore) {
		if (this->scope->score_network->last_update_iter != iter_index) {
			this->scope->score_network->update();

			this->scope->score_network->last_update_iter = iter_index;
		}
	}

	for (int h_index = (int)this->node_histories.size()-1; h_index >= 0; h_index--) {
		this->node_histories[h_index]->update(iter_index);
	}

	if (this->scope->obs_network->last_update_iter != iter_index) {
		this->scope->obs_network->update();

		this->scope->obs_network->last_update_iter = iter_index;
	}
}
