#include "branch_node.h"

#include "scope.h"
#include "score_network.h"

using namespace std;

void BranchNode::train_step(AbstractNodeHistory* history,
							Eigen::VectorXf& state,
							bool& hit_explore,
							TrainScopeHistory* train_scope_history) {
	if (hit_explore) {
		BranchNodeHistory* branch_node_history = (BranchNodeHistory*)history;

		TrainPredictBranchNodeHistory* train_history = new TrainPredictBranchNodeHistory(this);
		train_scope_history->node_histories.push_back(train_history);

		train_history->is_branch = branch_node_history->is_branch;
		this->branch_predict_network->activate(state);
		train_history->predict_branch_network_history = new ScoreNetworkHistory();
		this->branch_predict_network->save(train_history->predict_branch_network_history);
	}
}

void TrainPredictBranchNodeHistory::backprop(double target_val,
											 Eigen::VectorXf& state_error,
											 SolutionWrapper* wrapper) {
	BranchNode* branch_node = (BranchNode*)this->node;
	branch_node->branch_predict_network->load(this->predict_branch_network_history);
	if (this->is_branch) {
		if (branch_node->branch_predict_network->output->acti_vals(0) > 2.0) {
			branch_node->branch_predict_network->output->acti_vals(0) = 2.0;
		}
		branch_node->branch_predict_network->backprop(1.0,
													  state_error);
	} else {
		if (branch_node->branch_predict_network->output->acti_vals(0) < -2.0) {
			branch_node->branch_predict_network->output->acti_vals(0) = -2.0;
		}
		branch_node->branch_predict_network->backprop(-1.0,
													  state_error);
	}
}

void TrainPredictBranchNodeHistory::update(int iter_index) {
	BranchNode* branch_node = (BranchNode*)this->node;
	if (branch_node->branch_predict_network->last_update_iter != iter_index) {
		branch_node->branch_predict_network->update();

		branch_node->branch_predict_network->last_update_iter = iter_index;
	}
}
