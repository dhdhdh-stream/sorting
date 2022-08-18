#include "explore_node_append_jump.h"

using namespace std;

void ExploreNodeAppendJump::process() override {
	SolutionNodeIfStart* start_node = (SolutionNodeIfStart*)this->explore->solution->nodes[this->jump_start_node_index];
	SolutionNodeIfEnd* end_node = start_node->end;

	if (this->new_path_node_indexes.size() == 0) {
		start_node->children_nodes[this->new_child_index] = end_node;
	} else {
		SolutionNormalNode* curr_node = (SolutionNormalNode*)this->explore->solution->nodes[this->new_path_node_indexes[0]];
		start_node->children_nodes[this->new_child_index] = curr_node;
		curr_node->previous = start_node;
		for (int p_index = 1; p_index < (int)this->new_path_node_indexes.size(); p_index++) {
			SolutionNormalNode* next_node = (SolutionNormalNode*)this->explore->solution->nodes[this->new_path_node_indexes[p_index]];
			curr_node->next = next_node;
			next_node->previous = curr_node;
			curr_node = next_node;
		}
		curr_node->next = end_node;
	}
	start_node->children_on[this->new_child_index] = true;
}
