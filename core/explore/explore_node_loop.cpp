#include "explore_node_loop.h"

#include "explore_utilities.h"

using namespace std;

void ExploreNodeLoop::process(Solution* solution) override {
	SolutionNodeLoopStart* new_start_node = (SolutionNodeLoopStart*)solution->nodes[this->new_start_node_index];
	SolutionNodeLoopEnd* new_end_node = (SolutionNodeLoopEnd*)solution->nodes[this->new_end_node_index];

	SolutionNode* loop_start = solution->nodes[this->loop_start_node_index];
	SolutionNode* start_previous_previous;
	if (loop_start->type == NODE_TYPE_NORMAL) {
		SolutionNodeNormal* loop_start_normal = (SolutionNodeNormal*)loop_start;
		start_previous_previous = loop_start_normal->previous;
		loop_start_normal->previous = new_start_node;
	} else if (loop_start->type == NODE_TYPE_IF_START) {
		SolutionNodeIfStart* loop_start_if_start = (SolutionNodeIfStart*)loop_start;
		start_previous_previous = loop_start_if_start->previous;
		loop_start_if_start->previous = new_start_node;
	} else if (loop_start->type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* loop_start_loop_start = (SolutionNodeLoopStart*)loop_start;
		start_previous_previous = loop_start_loop_start->previous;
		loop_start_loop_start->previous = new_start_node;
	}
	new_start_node->next = loop_start;
	set_next(start_previous_previous, loop_start, new_start_node);
	new_start_node->previous = start_previous_previous;

	SolutionNode* loop_end = solution->nodes[this->loop_end_node_index];
	// can be equal to loop_start
	SolutionNode* end_previous_next;
	if (loop_end->type == NODE_TYPE_NORMAL) {
		SolutionNodeNormal* loop_end_normal = (SolutionNodeNormal*)loop_end;
		end_previous_next = loop_end_normal->next;
		loop_end_normal->next = new_end_node;
	} else if (loop_end->type == NODE_TYPE_IF_END) {
		SolutionNodeIfEnd* loop_end_if_end = (SolutionNodeIfEnd*)loop_end;
		end_previous_next = loop_end_if_end->next;
		loop_end_if_end->next = new_end_node;
	} else if (loop_end->type == NODE_TYPE_LOOP_END) {
		SolutionNodeLoopEnd* loop_end_loop_end = (SolutionNodeLoopEnd*)loop_end;
		end_previous_next = loop_end_loop_end->next;
		loop_end_loop_end->next = new_end_node;
	}
	new_end_node->next = end_previous_next;
	set_previous_if_needed(end_previous_next, new_end_node);

	if (this->new_path_node_indexes.size() == 0) {
		new_end_node->no_halt = new_start_node;
		new_start_node->loop_in = new_end_node;
	} else {
		SolutionNormalNode* curr_node = (SolutionNormalNode*)solution->nodes[this->new_path_node_indexes[0]];
		new_end_node->no_halt = curr_node;
		curr_node->previous = new_end_node;
		for (int p_index = 1; p_index < (int)this->new_path_node_indexes.size(); p_index++) {
			SolutionNodeNormal* next_node = (SolutionNodeNormal*)solution->nodes[this->new_path_node_indexes[p_index]];
			curr_node->next = next_node;
			next_node->previous = curr_node;
			curr_node = next_node;
		}
		curr_node->next = new_start_node;
		new_start_node->loop_in = curr_node;
	}
}
