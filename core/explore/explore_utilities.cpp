#include "explore_utilities.h"

#include "solution_node_action.h"
#include "solution_node_if_start.h"
#include "solution_node_if_end.h"
#include "solution_node_loop_start.h"
#include "solution_node_loop_end.h"

using namespace std;

void set_previous_if_needed(SolutionNode* node,
							SolutionNode* new_previous) {
	if (node->node_type == NODE_TYPE_IF_START) {
		SolutionNodeIfStart* node_if_start = (SolutionNodeIfStart*)node;
		node_if_start->previous = new_previous;
	} else if (node->node_type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* node_loop_start = (SolutionNodeLoopStart*)node;
		node_loop_start->previous = new_previous;
	} else if (node->node_type == NODE_TYPE_ACTION) {
		SolutionNodeAction* node_normal = (SolutionNodeAction*)node;
		node_normal->previous = new_previous;
	}
}

void set_next(SolutionNode* node,
			  SolutionNode* prev_next,
			  SolutionNode* new_next) {
	if (node->node_type == NODE_TYPE_IF_END) {
		SolutionNodeIfEnd* node_if_end = (SolutionNodeIfEnd*)node;
		node_if_end->next = new_next;
	} else if (node->node_type == NODE_TYPE_IF_START) {
		SolutionNodeIfStart* node_if_start = (SolutionNodeIfStart*)node;
		for (int c_index = 0; c_index < (int)node_if_start->children_nodes.size(); c_index++) {
			if (node_if_start->children_nodes[c_index] == prev_next) {
				node_if_start->children_nodes[c_index] = new_next;
				break;
			}
		}
	} else if (node->node_type == NODE_TYPE_LOOP_END) {
		SolutionNodeLoopEnd* node_loop_end = (SolutionNodeLoopEnd*)node;
		node_loop_end->next = new_next;
	} else if (node->node_type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* node_loop_start = (SolutionNodeLoopStart*)node;
		node_loop_start->next = new_next;
	} else if (node->node_type == NODE_TYPE_ACTION) {
		SolutionNodeAction* node_normal = (SolutionNodeAction*)node;
		node_normal->next = new_next;
	} else if (node->node_type == NODE_TYPE_START) {
		SolutionNodeStart* node_start = (SolutionNodeStart*)node;
		node_start->next = new_next;
	}
}
