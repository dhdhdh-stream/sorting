#include "explore_node_loop.h"

#include "explore_utilities.h"

using namespace std;

void ExploreNodeLoop::process() override {
	SolutionNodeLoopStart* new_start_node = (SolutionNodeLoopStart*)this->explore->solution->nodes[this->new_start_node_index];
	SolutionNodeLoopEnd* new_end_node = (SolutionNodeLoopEnd*)this->explore->solution->nodes[this->new_end_node_index];

	new_start_node->end = new_end_node;
	new_end_node->start = new_start_node;

	SolutionNode* loop_start_non_inclusive = this->explore->solution->nodes[this->loop_start_non_inclusive_index];
	new_start_node->previous = loop_start_non_inclusive;
	SolutionNode* loop_end_non_inclusive = this->explore->solution->nodes[this->loop_end_non_inclusive_index];
	new_end_node->next = loop_end_non_inclusive;
	set_previous_if_needed(loop_end_non_inclusive,
						   new_end_node);

	SolutionNode* loop_start_inclusive = this->explore->solution->nodes[this->loop_start_inclusive_index];
	SolutionNode* loop_end_inclusive = this->explore->solution->nodes[this->loop_end_inclusive_index];

	set_next(loop_start_non_inclusive,
			 loop_start_inclusive,
			 new_start_node);
	new_start_node->next = loop_start_inclusive;
	set_next(loop_end_inclusive,
			 loop_end_non_inclusive,
			 new_end_node);
}
