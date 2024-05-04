/**
 * TODO:
 * - potentially change to only InfoScopeNodes vs. only ActionNodes
 *   - only InfoScopeNodes = combining multiple previous checks
 *   - only ActionNodes = completely new check
 */

#include "solution_helpers.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "info_scope_node.h"
#include "problem.h"
#include "scope.h"

using namespace std;

Scope* create_new_info_scope() {
	Scope* new_scope = new Scope();

	new_scope->node_counter = 0;

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = new_scope;
	starting_noop_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[starting_noop_node->id] = starting_noop_node;
	starting_noop_node->action = Action(ACTION_NOOP);

	uniform_int_distribution<int> uniform_distribution(0, 1);
	geometric_distribution<int> geometric_distribution(0.5);
	int new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);

	uniform_int_distribution<int> default_distribution(0, 1);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		bool default_to_action = true;
		if (default_distribution(generator) != 0) {
			InfoScopeNode* new_scope_node = create_existing_info();
			if (new_scope_node != NULL) {
				new_scope_node->parent = new_scope;
				new_scope_node->id = new_scope->node_counter;
				new_scope->node_counter++;
				new_scope->nodes[new_scope_node->id] = new_scope_node;

				default_to_action = false;
			}
		}

		if (default_to_action) {
			ActionNode* new_action_node = new ActionNode();

			new_action_node->parent = new_scope;
			new_action_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = problem_type->random_action();
		}
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

		if (new_scope->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)new_scope->nodes[n_index];
			action_node->next_node_id = next_node_id;
			action_node->next_node = next_node;
		} else {
			InfoScopeNode* scope_node = (InfoScopeNode*)new_scope->nodes[n_index];
			scope_node->next_node_id = next_node_id;
			scope_node->next_node = next_node;
		}
	}

	return new_scope;
}
