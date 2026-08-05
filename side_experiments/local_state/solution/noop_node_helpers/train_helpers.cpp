#include "noop_node.h"

using namespace std;

void NoopNode::train_step(AbstractNodeHistory* history,
						  bool allow_drop,
						  Eigen::VectorXf& state,
						  TrainScopeHistory* train_scope_history) {
	// do nothing
}
