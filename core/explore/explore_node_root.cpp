#include "explore_node_root.h"

using namespace std;

ExploreNodeRoot::ExploreNodeRoot(Explore* explore) {
	this->explore = explore;

	this->type = EXPLORE_ROOT;

	this->count = 0;
	this->average_score = 0.0;
	this->average_misguess = 1.0;
}

ExploreNodeRoot::~ExploreNodeRoot() {
	// do nothing
}

void ExploreNodeRoot::process() override {
	SolutionNodeLoopStart* overall_start_node = (SolutionNodeLoopStart*)this->explore->solution->nodes[0];
	SolutionNodeLoopEnd* overall_end_node = (SolutionNodeLoopEnd*)this->explore->solution->nodes[1];

	overall_start_node->next = overall_end_node;
	overall_start_node->previous = NULL;
	overall_start_node->end = overall_end_node;

	overall_end_node->next = NULL;
	overall_end_node->start = overall_start_node;
}
