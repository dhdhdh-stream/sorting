#include "scope_node.h"

#include "constants.h"
#include "globals.h"
#include "predict_network.h"
#include "scope.h"
#include "transition_network.h"

using namespace std;

void ScopeNode::train_step(AbstractNodeHistory* history,
						   Eigen::VectorXf& state,
						   bool& is_done,
						   TrainScopeHistory* train_scope_history) {
	ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history;

	TrainScopeNodeHistory* train_history = new TrainScopeNodeHistory(this);
	train_scope_history->node_histories.push_back(train_history);

	TrainScopeHistory* inner_train_scope_history = new TrainScopeHistory(this->scope);
	train_history->scope_history = inner_train_scope_history;

	Eigen::VectorXf inner_state;
	inner_state.resize(NUM_STATES);
	inner_state.setConstant(0.0);

	this->in_network->activate(state,
							   inner_state);
	train_history->in_network_history = new TransitionNetworkHistory();
	this->in_network->save(train_history->in_network_history);

	this->scope->train_activate(scope_node_history->scope_history,
								inner_state,
								is_done,
								inner_train_scope_history);

	if (is_done) {
		train_history->early_exit = true;
	} else {
		train_history->early_exit = false;
		this->out_network->activate(inner_state,
									state);
		train_history->out_network_history = new TransitionNetworkHistory();
		this->out_network->save(train_history->out_network_history);
	}
}

void ScopeNode::train_predict_step(AbstractNodeHistory* history,
								   Eigen::VectorXf& state,
								   TrainScopeHistory* train_scope_history) {
	TrainPredictScopeNodeHistory* train_history = new TrainPredictScopeNodeHistory(this);
	train_scope_history->node_histories.push_back(train_history);

	this->predict_network->activate(state);
	train_history->predict_network_history = new PredictNetworkHistory();
	this->predict_network->save(train_history->predict_network_history);
}

void TrainScopeNodeHistory::backprop(double target_val,
									 Eigen::VectorXf& state_error) {
	ScopeNode* scope_node = (ScopeNode*)this->node;

	Eigen::VectorXf inner_state_error;
	inner_state_error.resize(NUM_STATES);
	inner_state_error.setConstant(0.0);

	if (!this->early_exit) {
		scope_node->out_network->load(this->out_network_history);
		scope_node->out_network->backprop(state_error,
										  inner_state_error);
	}

	this->scope_history->backprop(target_val,
								  state_error);

	scope_node->in_network->load(this->in_network_history);
	scope_node->in_network->backprop(inner_state_error,
									 state_error);
}

void TrainPredictScopeNodeHistory::backprop(double target_val,
											Eigen::VectorXf& state_error) {
	ScopeNode* scope_node = (ScopeNode*)this->node;
	scope_node->predict_network->load(this->predict_network_history);
	scope_node->predict_network->backprop(state_error);
}
