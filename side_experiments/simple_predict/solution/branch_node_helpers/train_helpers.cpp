#include "branch_node.h"

#include "scope.h"
#include "score_network.h"

using namespace std;

void BranchNode::train_step(AbstractNodeHistory* history,
							Eigen::VectorXf& state,
							bool& is_done,
							TrainScopeHistory* train_scope_history) {
	// do nothing
}

void BranchNode::train_predict_step(AbstractNodeHistory* history,
									Eigen::VectorXf& state,
									TrainScopeHistory* train_scope_history) {
	BranchNodeHistory* branch_node_history = (BranchNodeHistory*)history;

	TrainPredictBranchNodeHistory* train_history = new TrainPredictBranchNodeHistory(this);
	train_scope_history->node_histories.push_back(train_history);

	train_history->is_branch = branch_node_history->is_branch;
	this->branch_predict_network->activate(state);
	train_history->predict_branch_network_history = new ScoreNetworkHistory();
	this->branch_predict_network->save(train_history->predict_branch_network_history);
}

void TrainPredictBranchNodeHistory::backprop(double target_val,
											 Eigen::VectorXf& state_error) {
	BranchNode* branch_node = (BranchNode*)this->node;
	branch_node->branch_predict_network->load(this->predict_branch_network_history);
	branch_node->branch_predict_network->backprop(this->is_branch,
												  state_error);
}
