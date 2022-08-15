#include "explore_node_jump.h"

#include "explore_utilities.h"

using namespace std;

void ExploreNodeJump::process(Solution* solution) override {
	SolutionNodeIfStart* new_start_node = (SolutionNodeIfStart*)solution->nodes[this->new_start_node_index];
	SolutionNodeIfEnd* new_end_node = (SolutionNodeIfEnd*)solution->nodes[this->new_end_node_index];

	SolutionNode* jump_start = solution->nodes[this->jump_start_node_index];
	SolutionNode* start_prev_next;
	if (jump_start->type == NODE_TYPE_NORMAL) {
		SolutionNodeNormal* jump_start_normal = (SolutionNodeNormal*)jump_start;
		start_prev_next = jump_start_normal->next;
		jump_start_normal->next = new_start_node;
	} else if (jump_start->type == NODE_TYPE_IF_END) {
		SolutionNodeIfEnd* jump_start_if_end = (SolutionNodeIfEnd*)jump_start;
		start_prev_next = jump_start_if_end->next;
		jump_start_if_end->next = new_start_node;
	} else if (jump_start->type == NODE_TYPE_LOOP_END) {
		SolutionNodeLoopEnd* jump_start_loop_end = (SolutionNodeLoopEnd*)jump_start;
		start_prev_next = jump_start_loop_end->next;
		jump_start_loop_end->next = new_start_node;
	} else if (jump_start->type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* jump_start_loop_start = (SolutionNodeLoopStart*)jump_start;
		start_prev_next = jump_start_loop_start->next;
		jump_start_loop_start->next = new_start_node;
	}
	new_start_node->previous = jump_start;

	if (this->new_path_node_indexes.size() == 0) {
		new_start_node->children_nodes[1] = new_end_node;
	} else {
		SolutionNormalNode* curr_node = (SolutionNormalNode*)solution->nodes[this->new_path_node_indexes[0]];
		new_start_node->children_nodes[1] = curr_node;
		curr_node->previous = new_start_node;
		for (int p_index = 1; p_index < (int)this->new_path_node_indexes.size(); p_index++) {
			SolutionNormalNode* next_node = (SolutionNormalNode*)solution->nodes[this->new_path_node_indexes[p_index]];
			curr_node->next = next_node;
			next_node->previous = curr_node;
			curr_node = next_node;
		}
		curr_node->next = new_end_node;
	}

	if (jump_start_node_index == jump_end_node_index) {
		new_start_node->children_nodes[0] = new_end_node;
		new_end_node->next = start_prev_next;
		set_previous_if_needed(start_prev_next, new_end_node);
	} else {
		new_start_node->children_nodes[0] = start_prev_next;
		SolutionNode* jump_end = solution->nodes[this->jump_end_node_index];
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
}
