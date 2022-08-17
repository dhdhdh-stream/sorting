#include "explore_node_if_start_jump.h"

using namespace std;

void ExploreNodeIfStartJump::process() override {
	SolutionNodeIfStart* start_node = (SolutionNodeIfStart*)this->explore->solution->nodes[this->jump_start_node_index];
	if (this->if_start_jump_type == IF_START_JUMP_TYPE_END) {
		SolutionNodeIfEnd* end_node = (SolutionNodeIfEnd*)this->explore->solution->nodes[start_node->end_index];

		if (this->new_path_node_indexes.size() == 0) {
			start_node->children_nodes[this->new_child_index] = end_node;
			start_node->children_on[this->new_child_index] = true;
		} else {
			SolutionNormalNode* curr_node = (SolutionNormalNode*)this->explore->solution->nodes[this->new_path_node_indexes[0]];
			start_node->children_nodes[this->new_child_index] = curr_node;
			start_node->children_on[this->new_child_index] = true;
			curr_node->previous = start_node;
			for (int p_index = 1; p_index < (int)this->new_path_node_indexes.size(); p_index++) {
				SolutionNormalNode* next_node = (SolutionNormalNode*)this->explore->solution->nodes[this->new_path_node_indexes[p_index]];
				curr_node->next = next_node;
				next_node->previous = curr_node;
				curr_node = next_node;
			}
			curr_node->next = end_node;
		}
	} else {
		SolutionNodeIfStart* new_start_node = (SolutionNodeIfStart*)this->explore->solution->nodes[this->new_start_node_index];
		SolutionNodeIfEnd* new_end_node = (SolutionNodeIfEnd*)this->explore->solution->nodes[this->new_end_node_index];

		new_start_node->end = new_end_node;
		new_end_node->start = new_start_node;

		new_start_node->previous = start_node;

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

		if (jump_start_node_index == jump_end_node_index) {
			new_start_node->children_nodes[0] = new_end_node;
			new_end_node->next = start_node->children_nodes[this->modify_child_index];
		} else {
			new_start_node->children_nodes[0] = start_node->children_nodes[this->modify_child_index];
			SolutionNode* jump_end = this->explore->solution->nodes[this->jump_end_node_index];
			SolutionNode* end_previous_next;
			if (jump_end->type == NODE_TYPE_NORMAL) {
				SolutionNodeNormal* jump_end_normal = (SolutionNodeNormal*)jump_end;
				end_previous_next = jump_end_normal->next;
				jump_end_normal->next = new_end_node;
			} else if (jump_end->type == NODE_TYPE_IF_END) {
				SolutionNodeIfEnd* jump_end_if_end = (SolutionNodeIfEnd*)jump_end;
				end_previous_next = jump_end_if_end->next;
				jump_end_if_end->next = new_end_node;
			} else if (jump_end->type == NODE_TYPE_LOOP_END) {
				SolutionNodeLoopEnd* jump_end_loop_end = (SolutionNodeLoopEnd*)jump_end;
				end_previous_next = jump_end_loop_end->next;
				jump_end_loop_end->next = new_end_node;
			}
			new_end_node->next = end_previous_next;
			set_previous_if_needed(end_previous_next, new_end_node);
		}

		start_node->children_nodes[this->modify_child_index] = new_start_node;
	}
}
