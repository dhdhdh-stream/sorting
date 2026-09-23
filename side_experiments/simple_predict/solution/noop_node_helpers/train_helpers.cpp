#include "noop_node.h"

using namespace std;

void NoopNode::train_step(AbstractNodeHistory* history,
						  Eigen::VectorXf& state,
						  bool& is_done,
						  TrainScopeHistory* train_scope_history) {
	// do nothing
}

void NoopNode::train_predict_step(AbstractNodeHistory* history,
								  Eigen::VectorXf& state,
								  TrainScopeHistory* train_scope_history) {
	// do nothing
}
