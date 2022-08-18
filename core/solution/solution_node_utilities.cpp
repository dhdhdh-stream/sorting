#include "solution_node_utilities.h"

using namespace std;

void find_scope_end(SolutionNode* inclusive_start,
					SolutionNode*& inclusive_end,
					SolutionNode*& non_inclusive_end) {
	SolutionNode* curr_node = inclusive_start;
	while (true) {
		SolutionNode* next_node;
		if (curr_node->type == NODE_TYPE_NORMAL) {
			SolutionNodeNormal* curr_node_normal = (SolutionNodeNormal*)curr_node;
			next_node = curr_node_normal->next;
		} else if (curr_node->type == NODE_TYPE_IF_END) {
			SolutionNodeIfEnd* curr_node_if_end = (SolutionNodeIfEnd*)curr_node;
			next_node = curr_node_if_end->next;
		} else if (curr_node->type == NODE_TYPE_LOOP_END) {
			SolutionNodeLoopEnd* curr_node_loop_end = (SolutionNodeLoopEnd*)curr_node;
			next_node = curr_node_loop_end->halt;
		}

		if (next_node->type == NODE_TYPE_IF_END) {
			inclusive_end = curr_node;
			non_inclusive_end = next_node;
			return;
		}
		if (next_node->type == NODE_TYPE_LOOP_END) {
			inclusive_end = curr_node;
			non_inclusive_end = next_node;
			return;
		}
		if (next_node->type == NODE_TYPE_LOOP_START) {
			SolutionNodeLoopStart* next_node_loop_start = (SolutionNodeLoopStart*)next_node;
			if (next_node_loop_start->loop_in == curr_node) {
				inclusive_end = curr_node;
				non_inclusive_end = next_node;
				return;
			}
		}

		if (next_node->type == NODE_TYPE_NORMAL) {
			curr_node = next_node;
		} else if (next_node->type == NODE_TYPE_IF_START) {
			SolutionNodeIfStart* next_node_if_start = (SolutionNodeIfStart*)next_node;
			curr_node = next_node_if_start->end;
		} else if (next_node->type == NODE_TYPE_LOOP_START) {
			SolutionNodeLoopStart* next_node_loop_start = (SolutionNodeLoopStart*)next_node;
			curr_node = next_node_loop_start->end;
		}
	}
}

void find_potential_jumps(SolutionNode* inclusive_start,
						  vector<SolutionNode*>& potential_inclusive_jump_ends,
						  vector<SolutionNode*>& potential_non_inclusive_jump_ends) {
	SolutionNode* curr_node = inclusive_start;
	while (true) {
		SolutionNode* next_node;
		if (curr_node->type == NODE_TYPE_NORMAL) {
			SolutionNodeNormal* curr_node_normal = (SolutionNodeNormal*)curr_node;
			next_node = curr_node_normal->next;
		} else if (curr_node->type == NODE_TYPE_IF_END) {
			SolutionNodeIfEnd* curr_node_if_end = (SolutionNodeIfEnd*)curr_node;
			next_node = curr_node_if_end->next;
		} else if (curr_node->type == NODE_TYPE_LOOP_END) {
			SolutionNodeLoopEnd* curr_node_loop_end = (SolutionNodeLoopEnd*)curr_node;
			next_node = curr_node_loop_end->halt;
		}

		potential_inclusive_jump_ends.push_back(curr_node);
		potential_non_inclusive_jump_ends.push_back(next_node);

		if (next_node->type == NODE_TYPE_IF_END) {
			return;
		}
		if (next_node->type == NODE_TYPE_LOOP_END) {
			return;
		}
		if (next_node->type == NODE_TYPE_LOOP_START) {
			SolutionNodeLoopStart* next_node_loop_start = (SolutionNodeLoopStart*)next_node;
			if (next_node_loop_start->loop_in == curr_node) {
				return;
			}
		}

		if (next_node->type == NODE_TYPE_NORMAL) {
			curr_node = next_node;
		} else if (next_node->type == NODE_TYPE_IF_START) {
			SolutionNodeIfStart* next_node_if_start = (SolutionNodeIfStart*)next_node;
			curr_node = next_node_if_start->end;
		} else if (next_node->type == NODE_TYPE_LOOP_START) {
			SolutionNodeLoopStart* next_node_loop_start = (SolutionNodeLoopStart*)next_node;
			curr_node = next_node_loop_start->end;
		}
	}
}

void find_potential_loops(SolutionNode* inclusive_end,
						  vector<SolutionNode*>& potential_non_inclusive_loop_starts,
						  vector<SolutionNode*>& potential_inclusive_loop_starts) {
	SolutionNode* curr_node = inclusive_end;
	while (true) {
		SolutionNode* previous_node;
		if (curr_node->type == NODE_TYPE_NORMAL) {
			SolutionNodeNormal* curr_node_normal = (SolutionNodeNormal*)curr_node;
			previous_node = curr_node_normal->previous;
		} else if (curr_node->type == NODE_TYPE_IF_END) {
			SolutionNodeIfEnd* curr_node_if_end = (SolutionNodeIfEnd*)curr_node;
			SolutionNodeIfStart* scope_start = curr_node_if_end->start;
			previous_node = scope_start->previous;
		} else if (curr_node->type == NODE_TYPE_LOOP_END) {
			SolutionNodeLoopEnd* curr_node_loop_end = (SolutionNodeLoopEnd*)curr_node;
			SolutionNodeLoopStart* scope_start = curr_node_loop_end->start;
			previous_node = scope_start->previous;
		}

		potential_non_inclusive_loop_starts.push_back(previous_node);
		potential_inclusive_loop_starts.push_back(curr_node);

		if (previous_node->type == NODE_TYPE_IF_START) {
			return;
		}
		if (previous_node->type == NODE_TYPE_LOOP_START) {
			return;
		}
		if (previous_node->type == NODE_TYPE_LOOP_END) {
			SolutionNodeLoopEnd* previous_node_loop_end = (SolutionNodeLoopEnd*)previous_node;
			if (previous_node_loop_end->no_halt == curr_node) {
				return;
			}
		}

		curr_node = previous_node;
	}
}
