#include "scope_node.h"

#include "constants.h"
#include "globals.h"
#include "predict_network.h"
#include "scope.h"

using namespace std;

void ScopeNode::train_step(AbstractNodeHistory* history,
						   Eigen::VectorXf& state,
						   bool& hit_explore,
						   TrainScopeHistory* train_scope_history) {
	if (hit_explore) {
		TrainPredictScopeNodeHistory* train_history = new TrainPredictScopeNodeHistory(this);
		train_scope_history->node_histories.push_back(train_history);

		this->predict_network->activate(state);
		train_history->predict_network_history = new PredictNetworkHistory();
		this->predict_network->save(train_history->predict_network_history);
	} else {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history;

		TrainScopeNodeHistory* train_history = new TrainScopeNodeHistory(this);
		train_scope_history->node_histories.push_back(train_history);

		TrainScopeHistory* inner_train_scope_history = new TrainScopeHistory(this->scope);
		train_history->scope_history = inner_train_scope_history;

		this->scope->train_activate(scope_node_history->scope_history,
									state,
									hit_explore,
									inner_train_scope_history);
	}
}

void TrainScopeNodeHistory::backprop(double target_val,
									 Eigen::VectorXf& state_error,
									 SolutionWrapper* wrapper) {
	this->scope_history->backprop(target_val,
								  state_error,
								  wrapper);
}

void TrainPredictScopeNodeHistory::backprop(double target_val,
											Eigen::VectorXf& state_error,
											SolutionWrapper* wrapper) {
	ScopeNode* scope_node = (ScopeNode*)this->node;
	scope_node->predict_network->load(this->predict_network_history);
	scope_node->predict_network->backprop(state_error);
}

void TrainScopeNodeHistory::update(int iter_index) {
	this->scope_history->update(iter_index);
}

void TrainPredictScopeNodeHistory::update(int iter_index) {
	ScopeNode* scope_node = (ScopeNode*)this->node;
	if (scope_node->predict_network->last_update_iter != iter_index) {
		scope_node->predict_network->update();

		scope_node->predict_network->last_update_iter = iter_index;
	}
}
