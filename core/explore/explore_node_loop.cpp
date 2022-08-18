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
	new_end_node->halt = loop_end_non_inclusive;
	set_previous_if_needed(loop_end_non_inclusive,
						   new_end_node);

	if (loop_start_inclusive_index == -1) {	// && loop_end_inclusive_index == -1
		set_next(loop_start_non_inclusive,
				 loop_end_non_inclusive,
				 new_start_node);
		new_start_node->next = new_end_node;
	} else {
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

	if (this->new_path_node_indexes.size() == 0) {
		new_end_node->no_halt = new_start_node;
		new_start_node->loop_in = new_end_node;
	} else {
		SolutionNormalNode* curr_node = (SolutionNormalNode*)this->explore->solution->nodes[this->new_path_node_indexes[0]];
		new_end_node->no_halt = curr_node;
		curr_node->previous = new_end_node;
		for (int p_index = 1; p_index < (int)this->new_path_node_indexes.size(); p_index++) {
			SolutionNodeNormal* next_node = (SolutionNodeNormal*)this->explore->solution->nodes[this->new_path_node_indexes[p_index]];
			curr_node->next = next_node;
			next_node->previous = curr_node;
			curr_node = next_node;
		}
		curr_node->next = new_start_node;
		new_start_node->loop_in = curr_node;
	}
}
