#include "action_node.h"

#include "globals.h"
#include "obs_network.h"
#include "predict_network.h"
#include "scope.h"

using namespace std;

void ActionNode::train_step(AbstractNodeHistory* history,
							Eigen::VectorXf& state,
							bool& hit_explore,
							TrainScopeHistory* train_scope_history) {
	if (hit_explore) {
		TrainPredictActionNodeHistory* train_history = new TrainPredictActionNodeHistory(this);
		train_scope_history->node_histories.push_back(train_history);

		this->predict_network->activate(state);
		train_history->predict_network_history = new PredictNetworkHistory();
		this->predict_network->save(train_history->predict_network_history);
	} else {
		ActionNodeHistory* action_node_history = (ActionNodeHistory*)history;

		TrainActionNodeHistory* train_history = new TrainActionNodeHistory(this);
		train_scope_history->node_histories.push_back(train_history);

		this->obs_network->activate(state,
									action_node_history->obs);
		train_history->obs_network_history = new ObsNetworkHistory();
		this->obs_network->save(train_history->obs_network_history);
	}
}

void TrainActionNodeHistory::backprop(double target_val,
									  Eigen::VectorXf& state_error,
									  SolutionWrapper* wrapper) {
	ActionNode* action_node = (ActionNode*)this->node;
	action_node->obs_network->load(this->obs_network_history);
	action_node->obs_network->backprop(state_error);
}

void TrainPredictActionNodeHistory::backprop(double target_val,
											 Eigen::VectorXf& state_error,
											 SolutionWrapper* wrapper) {
	ActionNode* action_node = (ActionNode*)this->node;
	action_node->predict_network->load(this->predict_network_history);
	action_node->predict_network->backprop(state_error);
}

void TrainActionNodeHistory::update(int iter_index) {
	ActionNode* action_node = (ActionNode*)this->node;
	if (action_node->obs_network->last_update_iter != iter_index) {
		action_node->obs_network->update();

		action_node->obs_network->last_update_iter = iter_index;
	}
}

void TrainPredictActionNodeHistory::update(int iter_index) {
	ActionNode* action_node = (ActionNode*)this->node;
	if (action_node->predict_network->last_update_iter != iter_index) {
		action_node->predict_network->update();

		action_node->predict_network->last_update_iter = iter_index;
	}
}
