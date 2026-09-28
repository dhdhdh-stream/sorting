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
						   bool& hit_explore,
						   TrainScopeHistory* train_scope_history) {
	if (history->explore_index == -1) {
		for (int h_index = 0; h_index < (int)history->node_histories.size(); h_index++) {
			AbstractNode* node = history->node_histories[h_index]->node;
			node->train_step(history->node_histories[h_index],
							 state,
							 hit_explore,
							 train_scope_history);
		}
	} else {
		for (int h_index = 0; h_index <= history->explore_index; h_index++) {
			AbstractNode* node = history->node_histories[h_index]->node;
			node->train_step(history->node_histories[h_index],
							 state,
							 hit_explore,
							 train_scope_history);
		}

		hit_explore = true;

		for (int h_index = history->explore_index+1; h_index < (int)history->node_histories.size(); h_index++) {
			AbstractNode* node = history->node_histories[h_index]->node;
			node->train_step(history->node_histories[h_index],
							 state,
							 hit_explore,
							 train_scope_history);
		}
	}
}

void TrainScopeHistory::backprop(double target_val,
								 Eigen::VectorXf& state_error,
								 SolutionWrapper* wrapper) {
	for (int h_index = (int)this->node_histories.size()-1; h_index >= 0; h_index--) {
		this->node_histories[h_index]->backprop(target_val,
												state_error,
												wrapper);
	}
}

void TrainScopeHistory::update(int iter_index) {
	for (int h_index = (int)this->node_histories.size()-1; h_index >= 0; h_index--) {
		this->node_histories[h_index]->update(iter_index);
	}
}
