#include "explore_node_root.h"

using namespace std;

void ExploreNodeRoot::process() override {
	for (int n_index = 0; n_index < (int)this->explore->solution->nodes.size(); n_index++) {
		this->explore->solution->nodes[n_index]->reset();
	}

	SolutionNodeLoopStart* overall_start_node = (SolutionNodeLoopStart*)this->explore->solution->nodes[0];
	SolutionNodeLoopEnd* overall_end_node = (SolutionNodeLoopEnd*)this->explore->solution->nodes[1];

	overall_start_node->next = overall_end_node;
	overall_start_node->previous = NULL;
	overall_start_node->end = overall_end_node;

	overall_end_node->next = NULL;
	overall_end_node->start = overall_start_node;
}
