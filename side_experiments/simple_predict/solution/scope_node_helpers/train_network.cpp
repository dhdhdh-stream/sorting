#include "scope_node.h"

#include "constants.h"
#include "globals.h"
#include "predict_network.h"
#include "scope.h"
#include "transition_network.h"

using namespace std;

void ScopeNode::train_step(AbstractNodeHistory* history,
						   Eigen::VectorXf& state,
						   vector<TrainAbstractNodeHistory*>& train_node_histories) {
	ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history;

	TrainScopeNodeHistory* train_history = new TrainScopeNodeHistory(this);
	train_node_histories.push_back(train_history);

	this->out_network->activate(scope_node_history->end_inner_state,
								state);
	train_history->out_network_history = new TransitionNetworkHistory();
	this->out_network->save(train_history->out_network_history);
}

void ScopeNode::train_predict_step(AbstractNodeHistory* history,
								   Eigen::VectorXf& state,
								   vector<TrainAbstractNodeHistory*>& train_node_histories) {
	TrainPredictScopeNodeHistory* train_history = new TrainPredictScopeNodeHistory(this);
	train_node_histories.push_back(train_history);

	this->predict_network->activate(state);
	train_history->predict_network_history = new PredictNetworkHistory();
	this->predict_network->save(train_history->predict_network_history);
}

void TrainScopeNodeHistory::backprop(Eigen::VectorXf& state_error) {
	ScopeNode* scope_node = (ScopeNode*)this->node;
	scope_node->out_network->load(this->out_network_history);
	scope_node->out_network->backprop(state_error);
}

void TrainPredictScopeNodeHistory::backprop(Eigen::VectorXf& state_error) {
	ScopeNode* scope_node = (ScopeNode*)this->node;
	scope_node->predict_network->load(this->predict_network_history);
	scope_node->predict_network->backprop(state_error);
}

void TrainScopeNodeHistory::update(int iter_index) {
	ScopeNode* scope_node = (ScopeNode*)this->node;
	if (scope_node->out_network->last_update_iter != iter_index) {
		scope_node->out_network->update();

		scope_node->out_network->last_update_iter = iter_index;
	}
}

void TrainPredictScopeNodeHistory::update(int iter_index) {
	ScopeNode* scope_node = (ScopeNode*)this->node;
	if (scope_node->predict_network->last_update_iter != iter_index) {
		scope_node->predict_network->update();

		scope_node->predict_network->last_update_iter = iter_index;
	}
}
