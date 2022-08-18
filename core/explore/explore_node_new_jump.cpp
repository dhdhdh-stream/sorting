#include "explore_node_new_jump.h"

#include "explore_utilities.h"

using namespace std;

void ExploreNodeNewJump::process() override {
	SolutionNodeIfStart* new_start_node = (SolutionNodeIfStart*)this->explore->solution->nodes[this->new_start_node_index];
	SolutionNodeIfEnd* new_end_node = (SolutionNodeIfEnd*)this->explore->solution->nodes[this->new_end_node_index];

	new_start_node->end = new_end_node;
	new_end_node->start = new_start_node;

	SolutionNode* jump_start_non_inclusive = this->explore->solution->nodes[this->jump_start_non_inclusive_index];
	new_start_node->previous = jump_start_non_inclusive;
	SolutionNode* jump_end_non_inclusive = this->explore->solution->nodes[this->jump_end_non_inclusive_index];
	new_end_node->next = jump_end_non_inclusive;
	set_previous_if_needed(jump_end_non_inclusive,
						   new_end_node);

	if (jump_start_inclusive_index == -1) {	// && jump_end_inclusive_index == -1
		set_next(jump_start_non_inclusive,
				 jump_end_non_inclusive,
				 new_start_node);
		new_start_node->children_nodes[0] = new_end_node;
	} else {
		SolutionNode* loop_start_inclusive = this->explore->solution->nodes[this->loop_start_inclusive_index];
		SolutionNode* loop_end_inclusive = this->explore->solution->nodes[this->loop_end_inclusive_index];

		set_next(loop_start_non_inclusive,
				 loop_start_inclusive,
				 new_start_node);
		new_start_node->children_nodes[0] = loop_start_inclusive;
		set_next(loop_end_inclusive,
				 loop_end_non_inclusive,
				 new_end_node);
	}
	new_start_node->children_on[0] = true;

	if (this->new_path_node_indexes.size() == 0) {
		new_start_node->children_nodes[1] = new_end_node;
	} else {
		SolutionNormalNode* curr_node = (SolutionNormalNode*)this->explore->solution->nodes[this->new_path_node_indexes[0]];
		new_start_node->children_nodes[1] = curr_node;
		curr_node->previous = new_start_node;
		for (int p_index = 1; p_index < (int)this->new_path_node_indexes.size(); p_index++) {
			SolutionNormalNode* next_node = (SolutionNormalNode*)this->explore->solution->nodes[this->new_path_node_indexes[p_index]];
			curr_node->next = next_node;
			next_node->previous = curr_node;
			curr_node = next_node;
		}
		curr_node->next = new_end_node;
	}
	new_start_node->children_on[1] = true;
}
