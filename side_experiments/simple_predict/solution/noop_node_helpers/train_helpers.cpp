#include "noop_node.h"

using namespace std;

void NoopNode::train_step(AbstractNodeHistory* history,
						  Eigen::VectorXf& state,
						  vector<TrainAbstractNodeHistory*>& train_node_histories) {
	// do nothing
}

void NoopNode::train_predict_step(AbstractNodeHistory* history,
								  Eigen::VectorXf& state,
								  vector<TrainAbstractNodeHistory*>& train_node_histories) {
	// do nothing
}
