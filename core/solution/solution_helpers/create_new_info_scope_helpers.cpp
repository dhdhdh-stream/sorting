#include "solution_helpers.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "info_scope.h"
#include "info_scope_node.h"
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

	int new_num_existing = 0;

	vector<InfoScope*> possible_info_scopes;
	for (int i_index = 0; i_index < (int)solution->info_scopes.size(); i_index++) {
		possible_info_scopes.push_back(solution->info_scopes[i_index]);
	}
	if (possible_info_scopes.size() > 0) {
		/**
		 * - simply add existing info scopes to the front
		 */
		geometric_distribution<int> existing_distribution(0.5);
		new_num_existing = existing_distribution(generator);
		if (new_num_existing > (int)possible_info_scopes.size()) {
			new_num_existing = (int)possible_info_scopes.size();
		}

		for (int s_index = 0; s_index < new_num_existing; s_index++) {
			uniform_int_distribution<int> random_distribution(0, possible_info_scopes.size()-1);
			int random_index = random_distribution(generator);

			InfoScopeNode* new_scope_node = new InfoScopeNode();
			new_scope_node->scope = possible_info_scopes[random_index];

			new_scope_node->parent = new_scope;
			new_scope_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[new_scope_node->id] = new_scope_node;

			possible_info_scopes.erase(possible_info_scopes.begin() + random_index);
		}
	}

	uniform_int_distribution<int> uniform_distribution(0, 2);
	geometric_distribution<int> geometric_distribution(0.4);
	int new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);

	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		ActionNode* new_action_node = new ActionNode();

		new_action_node->parent = new_scope;
		new_action_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[new_action_node->id] = new_action_node;

		new_action_node->action = problem_type->random_action();
	}

	if (new_num_existing + new_num_steps > 0) {
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
