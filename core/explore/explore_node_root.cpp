#include "explore_node_root.h"

using namespace std;

void ExploreNodeRoot::process(Solution* solution) override {
	// TODO: clear everything

	SolutionNodeLoopStart* overall_start_node = (SolutionNodeLoopStart*)solution->nodes[0];
	SolutionNodeLoopEnd* overall_end_node = (SolutionNodeLoopEnd*)solution->nodes[1];

	overall_start_node->next = overall_end_node;
	overall_start_node->previous = NULL;

	overall_end_node->halt = NULL;
	overall_end_node->no_halt = NULL;
}
