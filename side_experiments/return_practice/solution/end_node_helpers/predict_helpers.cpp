#include "end_node.h"

#include "predict_run.h"

using namespace std;

void EndNode::predict_step(PredictRun* run) {
	run->node_context = NULL;
}
