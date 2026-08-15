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

	if (train_history->is_branch) {
		this->branch_network->activate(state);
		train_history->score_network_history = new ScoreNetworkHistory(this->branch_network);
		this->branch_network->save(train_history->score_network_history);
	} else {
		this->original_network->activate(state);
		train_history->score_network_history = new ScoreNetworkHistory(this->original_network);
		this->original_network->save(train_history->score_network_history);
	}
}
