#include "solution_node_utilities.h"

#include "solution_node_action.h"
#include "solution_node_if_start.h"
#include "solution_node_if_end.h"
#include "solution_node_loop_start.h"
#include "solution_node_loop_end.h"

using namespace std;

void new_random_scope(SolutionNode* explore_node,
					  int& parent_jump_scope_start_non_inclusive_index,
					  int& parent_jump_end_non_inclusive_index) {
	parent_jump_scope_start_non_inclusive_index = \
		-1 + rand()%(explore_node->scope_node_index+1);
	if (explore_node->parent_scope->node_type == NODE_TYPE_START_SCOPE) {
		parent_jump_end_non_inclusive_index = \
			explore_node->scope_node_index + 1 \
			+ rand()%(explore_node->parent_scope->path.size() - explore_node->scope_node_index);
	} else {
		// explore_node->parent_scope->node_type == NODE_TYPE_JUMP_SCOPE
		if (explore_node->scope_location == SCOPE_LOCATION_TOP) {
			parent_jump_end_non_inclusive_index = \
				explore_node->scope_node_index + 1 \
				+ rand()%(explore_node->parent_scope->top_path.size() - explore_node->scope_node_index);
		} else {
			parent_jump_end_non_inclusive_index = \
				explore_node->scope_node_index + 1 \
				+ rand()%(explore_node->parent_scope->children_nodes[
					explore_node->scope_child_index].size() - explore_node->scope_node_index);
		}
	}
}

void new_random_path(vector<SolutionNode*>& explore_path,
					 bool can_be_empty) {
	geometric_distribution<int> seq_length_dist(0.2);
	int seq_length;
	if (can_be_empty) {
		seq_length = seq_length_dist(generator);
	} {
		seq_length = 1 + seq_length_dist(generator);
	}

	if (seq_length == 0) {
		SolutionNodeEmpty* new_node = new SolutionNodeEmpty();
		explore_path.push_back(new_node);
	} else {
		for (int a_index = 0; a_index < seq_length; a_index++) {
			if (rand()%2 == 0 && action_dictionary->actions.size() > 0) {
				int random_dictionary_index = rand()%(int)action_dictionary->actions.size();
				vector<SolutionNode*> existing_action = action_dictionary->actions[random_dictionary_index];

				int subset_inclusive_start = rand()%(int)existing_action.size();
				int subset_non_inclusive_end = subset_start + 1 + rand()%((int)existing_action.size() - subset_start);
				for (int s_index = subset_inclusive_start; s_index < subset_non_inclusive_end; s_index++) {
					// SolutionNodeEmpty cannot be on top layer in action_dictionary
					if (existing_action[s_index]->node_type == SOLUTION_NODE_ACTION) {
						SolutionNodeAction* node_action = (SolutionNodeAction*)existing_action[s_index];
						SolutionNode* new_node = new SolutionNodeAction(node_action->action);
						explore_path.push_back(new_node);
					} else {
						explore_path.push_back(existing_action[s_index]);
					}
				}
			} else {
				normal_distribution<double> write_val_dist(0.0, 2.0);
				Action random_raw_action(write_val_dist(generator), rand()%3);
				SolutionNode* new_node = new SolutionNodeAction(random_raw_action);
				explore_path.push_back(new_node);
			}
		}

		for (int a_index = 0; a_index < seq_length-1; a_index++) {
			explore_path[a_index]->next = explore_path[a_index+1];
		}
	}
}

SolutionNode* get_jump_end(SolutionNode* explore_node) {
	if (explore_node->parent_scope->node_type == NODE_TYPE_START_SCOPE) {
		if (explore_node->parent_jump_end_non_inclusive_index \
				>= explore_node->parent_scope->path.size()) {
			return explore_node->parent_scope;
		} else {
			return explore_node->parent_scope[explore_node->parent_jump_end_non_inclusive_index];
		}
	} else {
		// explore_node->parent_scope->node_type == NODE_TYPE_JUMP_SCOPE
		if (explore_node->scope_location == SCOPE_LOCATION_TOP) {
			if (explore_node->parent_jump_end_non_inclusive_index \
					>= explore_node->parent_scope->top_path.size()) {
				return explore_node->parent_scope;
			} else {
				return explore_node->parent_scope->top_path[
					explore_node->parent_jump_end_non_inclusive_index];
			}
		} else {
			// explore_node->scope_location == SCOPE_LOCATION_BOTTOM
			if (explore_node->parent_jump_end_non_inclusive_index \
					>= explore_node->parent_scope->children_nodes[explore_node->scope_child_index].size()) {
				return explore_node->parent_scope;
			} else {
				return explore_node->parent_scope->children_nodes[
					explore_node->scope_child_index][explore_node->parent_jump_end_non_inclusive_index];
			}
		}
	}
}
