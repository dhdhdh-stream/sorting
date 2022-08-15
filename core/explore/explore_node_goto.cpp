#include "explore_node_goto.h"

using namespace std;

void ExploreNodeGoto::process(Solution* solution) override {
	SolutionNodeGotoStart* new_start_node = (SolutionNodeGotoStart*)solution->nodes[this->new_start_node_index];
	SolutionNodeGotoEnd* new_end_node = (SolutionNodeGotoEnd*)solution->nodes[this->new_end_node_index];

	if (this->new_path_node_indexes.size() == 0) {
		new_start_node->next = new_end_node;
	} else {
		SolutionNormalNode* curr_node = (SolutionNormalNode*)solution->nodes[this->new_path_node_indexes[0]];
		new_start_node->next = curr_node;
		curr_node->previous = new_start_node;
		for (int p_index = 1; p_index < (int)this->new_path_node_indexes.size(); p_index++) {
			SolutionNormalNode* next_node = (SolutionNormalNode*)solution->nodes[this->new_path_node_indexes[p_index]];
			curr_node->next = next_node;
			next_node->previous = curr_node;
			curr_node = next_node;
		}
		curr_node->next = new_end_node;
	}

	SolutionNode* node = solution->nodes[this->node_index];
	node->goto_on[this->goto_index] = true;
}
