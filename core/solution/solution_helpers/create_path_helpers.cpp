#include "solution_helpers.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

bool create_path(Scope* parent_scope,
				 RunHelper& run_helper,
				 vector<int>& step_types,
				 vector<ActionNode*>& actions,
				 vector<ScopeNode*>& scopes,
				 vector<std::set<int>>& catch_throw_ids) {
	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;

	vector<Scope*> scope_context{parent_scope};
	vector<AbstractNode*> node_context{NULL};

	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	int random_curr_depth = run_helper.curr_depth;
	int random_throw_id = -1;
	bool random_exceeded_limit = false;

	parent_scope->random_activate(parent_scope->default_starting_node,
								  scope_context,
								  node_context,
								  exit_depth,
								  exit_node,
								  random_curr_depth,
								  random_throw_id,
								  random_exceeded_limit,
								  possible_scope_contexts,
								  possible_node_contexts);

	if (possible_scope_contexts.size() == 0
			|| random_exceeded_limit) {
		return false;
	}

	uniform_int_distribution<int> start_distribution(0, possible_scope_contexts.size()-1);
	int start_index = start_distribution(generator);
	geometric_distribution<int> path_length_distribution(0.3);
	int end_index = start_index + path_length_distribution(generator);
	if (end_index > (int)possible_scope_contexts.size()-1) {
		end_index = (int)possible_scope_contexts.size()-1;
	}

	for (int n_index = start_index; n_index < end_index; n_index++) {
		if (possible_node_contexts[n_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* original_action_node = (ActionNode*)possible_node_contexts[n_index].back();

			if (original_action_node->action.move != ACTION_NOOP) {
				step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = original_action_node->action;
				actions.push_back(new_action_node);

				scopes.push_back(NULL);
				catch_throw_ids.push_back(set<int>());
			}
		} else if (possible_node_contexts[n_index].back()->type == NODE_TYPE_SCOPE) {
			step_types.push_back(STEP_TYPE_SCOPE);

			actions.push_back(NULL);

			ScopeNode* original_scope_node = (ScopeNode*)possible_node_contexts[n_index].back();
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->starting_node_id = original_scope_node->starting_node_id;
			new_scope_node->starting_node = original_scope_node->starting_node;
			new_scope_node->scope = original_scope_node->scope;
			scopes.push_back(new_scope_node);

			catch_throw_ids.push_back(set<int>());
		}
	}

	return true;
}
