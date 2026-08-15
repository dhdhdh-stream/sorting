#include "noop_node.h"

#include "scope.h"
#include "score_network.h"

using namespace std;

void NoopNode::train_step(AbstractNodeHistory* history,
						  bool allow_drop,
						  Eigen::VectorXf& state,
						  TrainScopeHistory* train_scope_history) {
	TrainNoopNodeHistory* train_history = new TrainNoopNodeHistory(this);
	train_scope_history->node_histories.push_back(train_history);

	// this->score_network->activate(state);
	// train_history->score_network_history = new ScoreNetworkHistory(this->score_network);
	// this->score_network->save(train_history->score_network_history);
}
