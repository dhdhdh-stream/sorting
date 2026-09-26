#include "noop_node.h"

using namespace std;

void NoopNode::train_step(AbstractNodeHistory* history,
						  Eigen::VectorXf& state,
						  bool& hit_explore,
						  TrainScopeHistory* train_scope_history) {
	// do nothing
}
