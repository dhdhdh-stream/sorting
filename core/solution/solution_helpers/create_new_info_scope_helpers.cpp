#include "solution_helpers.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "info_scope.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

InfoScope* create_new_info_scope() {
	InfoScope* new_scope = new InfoScope();

	new_scope->node_counter = 0;

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = new_scope;
	starting_noop_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[starting_noop_node->id] = starting_noop_node;
	starting_noop_node->action = Action(ACTION_NOOP);

	geometric_distribution<int> geometric_distribution(0.3);
	int new_num_steps = geometric_distribution(generator);

	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		ActionNode* new_action_node = new ActionNode();

		new_action_node->parent = new_scope;
		new_action_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[new_action_node->id] = new_action_node;

		new_action_node->action = problem_type->random_action();
	}

	if (new_num_steps > 0) {
		ActionNode* ending_node = new ActionNode();
		ending_node->parent = new_scope;
		ending_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[ending_node->id] = ending_node;
		ending_node->action = Action(ACTION_NOOP);
	}

	for (int n_index = 0; n_index < (int)new_scope->nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)new_scope->nodes.size()-1) {
			next_node_id = -1;
			next_node = NULL;
		} else {
			next_node_id = n_index+1;
			next_node = new_scope->nodes[n_index+1];
		}

		ActionNode* action_node = (ActionNode*)new_scope->nodes[n_index];
		action_node->next_node_id = next_node_id;
		action_node->next_node = next_node;
	}

	return new_scope;
}
