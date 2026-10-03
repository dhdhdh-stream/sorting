#include "branch_node.h"

#include "scope.h"
#include "score_network.h"

using namespace std;

void BranchNode::train_step(AbstractNodeHistory* history,
							Eigen::VectorXf& state,
							vector<TrainAbstractNodeHistory*>& train_node_histories) {
	// do nothing
}

void BranchNode::train_predict_step(AbstractNodeHistory* history,
									Eigen::VectorXf& state,
									vector<TrainAbstractNodeHistory*>& train_node_histories) {
	BranchNodeHistory* branch_node_history = (BranchNodeHistory*)history;

	TrainPredictBranchNodeHistory* train_history = new TrainPredictBranchNodeHistory(this);
	train_node_histories.push_back(train_history);

	train_history->is_branch = branch_node_history->is_branch;
	this->branch_predict_network->activate(state);
}

void TrainPredictBranchNodeHistory::backprop(Eigen::VectorXf& state_error) {
	BranchNode* branch_node = (BranchNode*)this->node;
	if (this->is_branch) {
		branch_node->branch_predict_network->backprop(1.0,
													  state_error);
	} else {
		branch_node->branch_predict_network->backprop(-1.0,
													  state_error);
	}
}

void BranchNode::update_step(int iter_index) {
	// do nothing
}

void BranchNode::update_predict_step(int iter_index) {
	if (this->branch_predict_network->last_update_iter != iter_index) {
		this->branch_predict_network->update();

		this->branch_predict_network->last_update_iter = iter_index;
	}
}
