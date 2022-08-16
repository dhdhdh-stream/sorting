#include "explore_node_root.h"

using namespace std;

void ExploreNodeRoot::process(Solution* solution) override {
	for (int n_index = 0; n_index < (int)solution->nodes.size(); n_index++) {
		solution->nodes[n_index]->reset();
	}

	SolutionNodeLoopStart* overall_start_node = (SolutionNodeLoopStart*)solution->nodes[0];
	SolutionNodeLoopEnd* overall_end_node = (SolutionNodeLoopEnd*)solution->nodes[1];

	overall_start_node->next = overall_end_node;
	overall_start_node->previous = NULL;

	overall_end_node->halt = NULL;
	overall_end_node->no_halt = NULL;
}
