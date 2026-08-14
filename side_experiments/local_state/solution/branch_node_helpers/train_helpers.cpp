#include "branch_node.h"

#include "scope.h"
#include "score_network.h"

using namespace std;

void BranchNode::train_step(AbstractNodeHistory* history,
							bool allow_drop,
							Eigen::VectorXf& state,
							TrainScopeHistory* train_scope_history) {
	BranchNodeHistory* branch_node_history = (BranchNodeHistory*)history;

	TrainBranchNodeHistory* train_history = new TrainBranchNodeHistory(this);
	train_scope_history->node_histories.push_back(train_history);

	train_history->is_branch = branch_node_history->is_branch;

	this->original_network->activate(state);
	train_history->original_network_history = new ScoreNetworkHistory(this->original_network);
	this->original_network->save(train_history->original_network_history);

	this->branch_network->activate(state);
	train_history->branch_network_history = new ScoreNetworkHistory(this->branch_network);
	this->branch_network->save(train_history->branch_network_history);

	this->preserve_original_network->activate(state);
	train_history->preserve_original_network_val = this->preserve_original_network->output->acti_vals(0);

	this->preserve_branch_network->activate(state);
	train_history->preserve_branch_network_val = this->preserve_branch_network->output->acti_vals(0);
}
