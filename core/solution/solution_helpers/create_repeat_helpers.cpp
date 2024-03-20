/**
 * - create scopes on successful repeat as they are "stable"
 *   - i.e., end in a way that allows follow-up
 */

#include "solution_helpers.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void create_repeat_helper(int target_depth,
						  vector<Scope*>& scope_context,
						  vector<AbstractNode*>& node_context,
						  vector<vector<Scope*>>& possible_scope_contexts,
						  vector<vector<AbstractNode*>>& possible_node_contexts,
						  ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;

			/**
			 * - decision making tied to original nodes will break anyways, so don't include NOOPs
			 */
			if (action_node->action.move != ACTION_NOOP) {
				node_context.back() = action_node;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);

				node_context.back() = NULL;
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			if (h_index == (int)scope_history->node_histories.size()-1
					&& (int)scope_context.size()+1 <= target_depth) {
				create_repeat_helper(target_depth,
									 scope_context,
									 node_context,
									 possible_scope_contexts,
									 possible_node_contexts,
									 scope_node_history->scope_history);
			} else {
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);

				node_context.back() = NULL;
			}
		}
	}

	// no need to pop_back context
}

bool create_repeat(vector<ContextLayer>& context,
				   int explore_context_depth,
				   vector<int>& step_types,
				   vector<ActionNode*>& actions,
				   vector<ScopeNode*>& existing_scopes,
				   vector<ScopeNode*>& potential_scopes,
				   vector<std::set<int>>& catch_throw_ids) {
	ScopeHistory* scope_history = context[context.size() - explore_context_depth].scope_history;

	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;

	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	create_repeat_helper(explore_context_depth,
						 scope_context,
						 node_context,
						 possible_scope_contexts,
						 possible_node_contexts,
						 scope_history);

	if (possible_scope_contexts.size() == 0) {
		return false;
	}

	geometric_distribution<int> length_distribution(0.3);
	int repeat_length = 1 + length_distribution(generator);
	if (repeat_length > (int)possible_scope_contexts.size()) {
		repeat_length = (int)possible_scope_contexts.size();
	}
	int start_index = (int)possible_scope_contexts.size() - repeat_length;

	if (repeat_length == 1) {
		if (possible_node_contexts.back().back()->type == NODE_TYPE_ACTION) {
			ActionNode* original_action_node = (ActionNode*)possible_node_contexts.back().back();

			step_types.push_back(STEP_TYPE_ACTION);

			ActionNode* new_action_node = new ActionNode();
			new_action_node->action = original_action_node->action;
			actions.push_back(new_action_node);

			existing_scopes.push_back(NULL);
			potential_scopes.push_back(NULL);
			catch_throw_ids.push_back(set<int>());
		} else if (possible_node_contexts.back().back()->type == NODE_TYPE_SCOPE) {
			ScopeNode* original_scope_node = (ScopeNode*)possible_node_contexts.back().back();

			step_types.push_back(STEP_TYPE_EXISTING_SCOPE);

			actions.push_back(NULL);

			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->scope = original_scope_node->scope;
			new_scope_node->starting_node_id = original_scope_node->starting_node_id;
			new_scope_node->starting_node = original_scope_node->starting_node;
			new_scope_node->exit_node_ids = original_scope_node->exit_node_ids;
			new_scope_node->exit_nodes = original_scope_node->exit_nodes;
			existing_scopes.push_back(new_scope_node);

			potential_scopes.push_back(NULL);
			catch_throw_ids.push_back(set<int>());
		}

		return true;
	} else {
		Scope* new_scope = new Scope();
		// don't set id/increment scope_counter until train
		new_scope->node_counter = 0;
		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->scope = new_scope;

		vector<AbstractNode*> new_nodes;

		ActionNode* new_noop_action_node = new ActionNode();
		new_noop_action_node->action = Action(ACTION_NOOP);
		new_nodes.push_back(new_noop_action_node);

		for (int n_index = start_index; n_index < (int)possible_scope_contexts.size(); n_index++) {
			if (possible_node_contexts[n_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* original_action_node = (ActionNode*)possible_node_contexts[n_index].back();

				ActionNode* new_action_node = new ActionNode();

				new_action_node->action = original_action_node->action;

				new_nodes.push_back(new_action_node);
			} else if (possible_node_contexts[n_index].back()->type == NODE_TYPE_SCOPE) {
				ScopeNode* original_scope_node = (ScopeNode*)possible_node_contexts[n_index].back();

				ScopeNode* new_scope_node = new ScopeNode();

				new_scope_node->scope = original_scope_node->scope;
				new_scope_node->starting_node_id = original_scope_node->starting_node_id;
				new_scope_node->starting_node = original_scope_node->starting_node;
				new_scope_node->exit_node_ids = original_scope_node->exit_node_ids;
				new_scope_node->exit_nodes = original_scope_node->exit_nodes;

				new_nodes.push_back(new_scope_node);
			}
		}

		for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
			new_nodes[n_index]->parent = new_scope;
			new_nodes[n_index]->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[new_nodes[n_index]->id] = new_nodes[n_index];

			int next_node_id;
			AbstractNode* next_node;
			if (n_index == (int)new_nodes.size()-1) {
				next_node_id = -1;
				next_node = NULL;
			} else {
				next_node_id = n_index+1;
				next_node = new_nodes[n_index+1];
			}

			if (new_nodes[n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)new_nodes[n_index];
				action_node->next_node_id = next_node_id;
				action_node->next_node = next_node;
			} else {
				ScopeNode* scope_node = (ScopeNode*)new_nodes[n_index];
				scope_node->next_node_id = next_node_id;
				scope_node->next_node = next_node;
			}
		}

		new_scope->default_starting_node_id = 0;
		new_scope->default_starting_node = new_nodes[0];
		new_scope_node->starting_node_id = 0;
		new_scope_node->starting_node = new_nodes[0];

		step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);

		actions.push_back(NULL);
		existing_scopes.push_back(NULL);

		potential_scopes.push_back(new_scope_node);

		catch_throw_ids.push_back(set<int>());

		return true;
	}
}
