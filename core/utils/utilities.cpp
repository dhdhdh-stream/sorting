#include "utilities.h"

#include "definitions.h"

using namespace std;

int calculate_action_path_length(Action action) {
	if (action.move == COMPOUND) {
		CompoundAction* compound_action = action_dictionary->actions[action.index];

		int sum_length = 0;
		CompoundActionNode* curr_node = compound_action->nodes[1];
		while (true) {
			if (curr_node->children_indexes[0] == 0) {
				break;
			}

			Action curr_action = curr_node->children_actions[0];
			sum_length += calculate_action_path_length(curr_action);

			curr_node = compound_action->nodes[curr_node->children_indexes[0]];
		}

		return sum_length;
	} else if (action.move == LOOP) {
		Loop* loop = loop_dictionary->established[action.index];
		return loop->path_length();
	} else {
		return 1;
	}
}
