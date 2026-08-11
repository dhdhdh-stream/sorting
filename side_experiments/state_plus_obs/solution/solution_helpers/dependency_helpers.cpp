#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

void gather_dependencies_helper(ScopeHistory* scope_history,
								vector<int>& curr_context,
								vector<int>& curr_index,
								int& count,
								vector<int>& dependency,
								vector<int>& index) {
	{
		uniform_int_distribution<int> distribution(0, count);
		count++;
		if (distribution(generator) == 0) {
			curr_context.push_back(-1);
			curr_index.push_back(-1);
			dependency = curr_context;
			index = curr_index;
			curr_context.pop_back();
			curr_index.pop_back();
		}
	}

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNode* node = scope_history->node_histories[h_index]->node;
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				uniform_int_distribution<int> distribution(0, count);
				count++;
				if (distribution(generator) == 0) {
					curr_context.push_back(node->id);
					curr_index.push_back(h_index);
					dependency = curr_context;
					index = curr_index;
					curr_context.pop_back();
					curr_index.pop_back();
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[h_index];
				curr_context.push_back(node->id);
				curr_index.push_back(h_index);
				gather_dependencies_helper(scope_node_history->scope_history,
										   curr_context,
										   curr_index,
										   count,
										   dependency,
										   index);
				curr_context.pop_back();
				curr_index.pop_back();
			}
			break;
		}
	}
}

void gather_dependencies_top_helper(ScopeHistory* scope_history,
									int top_index,
									vector<int>& curr_context,
									vector<int>& curr_index,
									int& count,
									vector<int>& dependency,
									vector<int>& index) {
	{
		uniform_int_distribution<int> distribution(0, count);
		count++;
		if (distribution(generator) == 0) {
			curr_context.push_back(-1);
			curr_index.push_back(-1);
			dependency = curr_context;
			index = curr_index;
			curr_context.pop_back();
			curr_index.pop_back();
		}
	}

	for (int h_index = 0; h_index <= top_index; h_index++) {
		AbstractNode* node = scope_history->node_histories[h_index]->node;
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				uniform_int_distribution<int> distribution(0, count);
				count++;
				if (distribution(generator) == 0) {
					curr_context.push_back(node->id);
					curr_index.push_back(h_index);
					dependency = curr_context;
					index = curr_index;
					curr_context.pop_back();
					curr_index.pop_back();
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[h_index];
				curr_context.push_back(node->id);
				curr_index.push_back(h_index);
				gather_dependencies_helper(scope_node_history->scope_history,
										   curr_context,
										   curr_index,
										   count,
										   dependency,
										   index);
				curr_context.pop_back();
				curr_index.pop_back();
			}
			break;
		}
	}
}

void set_dependency_helper(Scope* scope,
						   vector<int>& dependency,
						   int l_index,
						   AbstractExperiment* experiment) {
	if (l_index == (int)dependency.size()-1) {
		if (dependency[l_index] == -1) {
			scope->dependencies.push_back(experiment);
		} else {
			ActionNode* action_node = (ActionNode*)scope->nodes[dependency[l_index]];
			action_node->dependencies.push_back(experiment);
		}
	} else {
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[dependency[l_index]];
		set_dependency_helper(scope_node->scope,
							  dependency,
							  l_index+1,
							  experiment);
	}
}

Scope* get_dependency_scope(Scope* scope,
							vector<int>& dependency,
							int l_index) {
	if (l_index == (int)dependency.size()-1) {
		return scope;
	} else {
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[dependency[l_index]];
		return get_dependency_scope(scope_node->scope,
									dependency,
									l_index+1);
	}
}

void get_dependency_changes_helper(Scope* scope,
								   vector<int>& dependency,
								   int l_index,
								   set<Scope*>& scopes_needed,
								   set<ScopeNode*>& transitions_needed) {
	if (l_index == (int)dependency.size()-1) {
		return;
	} else {
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[dependency[l_index]];

		scopes_needed.insert(scope_node->scope);
		transitions_needed.insert(scope_node);

		get_dependency_changes_helper(scope_node->scope,
									  dependency,
									  l_index+1,
									  scopes_needed,
									  transitions_needed);
	}
}

void clear_dependency_helper(Scope* scope,
							 vector<int>& dependency,
							 int l_index,
							 AbstractExperiment* experiment) {
	if (l_index == (int)dependency.size()-1) {
		if (dependency[l_index] == -1) {
			for (int d_index = 0; d_index < (int)scope->dependencies.size(); d_index++) {
				if (scope->dependencies[d_index] == experiment) {
					scope->dependencies.erase(scope->dependencies.begin() + d_index);
					break;
				}
			}
		} else {
			ActionNode* action_node = (ActionNode*)scope->nodes[dependency[l_index]];
			for (int d_index = 0; d_index < (int)action_node->dependencies.size(); d_index++) {
				if (action_node->dependencies[d_index] == experiment) {
					action_node->dependencies.erase(action_node->dependencies.begin() + d_index);
					break;
				}
			}
		}
	} else {
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[dependency[l_index]];
		clear_dependency_helper(scope_node->scope,
								dependency,
								l_index+1,
								experiment);
	}
}

void fetch_dependency_helper(ScopeHistory* scope_history,
							 vector<int>& dependency,
							 int l_index,
							 bool& is_hit,
							 Eigen::VectorXf& state,
							 vector<double>& obs) {
	if (dependency[l_index] == -1) {
		is_hit = true;
		state = scope_history->state;
		obs = scope_history->obs;
	} else {
		int index = -1;
		for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
			if (scope_history->node_histories[h_index]->node->id == dependency[l_index]) {
				index = h_index;
				break;
			}
		}

		if (index == -1) {
			is_hit = false;
		} else {
			if (l_index == (int)dependency.size()-1) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[index];
				is_hit = true;
				state = action_node_history->state;
				obs = action_node_history->obs;
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[index];
				fetch_dependency_helper(scope_node_history->scope_history,
										dependency,
										l_index+1,
										is_hit,
										state,
										obs);
			}
		}
	}
}

void add_dependency_helper(Scope* scope,
						   vector<Scope*>& init_network_scope_context,
						   vector<int>& dependency,
						   int l_index,
						   InitNetwork* init_network) {
	init_network_scope_context.push_back(scope);

	if (l_index == (int)dependency.size()-1) {
		if (dependency[l_index] == -1) {
			scope->start_init_network_scope_contexts.push_back(init_network_scope_context);
			scope->start_init_network_node_contexts.push_back(dependency);
			scope->start_init_networks.push_back(init_network);
		} else {
			ActionNode* action_node = (ActionNode*)scope->nodes[dependency[l_index]];
			action_node->init_network_scope_contexts.push_back(init_network_scope_context);
			action_node->init_network_node_contexts.push_back(dependency);
			action_node->init_networks.push_back(init_network);
		}
	} else {
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[dependency[l_index]];
		add_dependency_helper(scope_node->scope,
							  init_network_scope_context,
							  dependency,
							  l_index+1,
							  init_network);
	}
}

bool match_dependency_helper(SolutionWrapper* wrapper,
							 vector<Scope*>& scope_contexts,
							 vector<int>& node_contexts) {
	if (wrapper->scope_histories.size() < scope_contexts.size()) {
		return false;
	} else {
		bool is_match = true;
		for (int l_index = 0; l_index < (int)scope_contexts.size()-1; l_index++) {
			int index = (int)wrapper->scope_histories.size() - (int)scope_contexts.size() + l_index;
			if (wrapper->scope_histories[index]->scope != scope_contexts[l_index]) {
				is_match = false;
				break;
			}
			if (wrapper->node_context[index] == NULL
					|| wrapper->node_context[index]->id != node_contexts[l_index]) {
				is_match = false;
				break;
			}
		}
		return is_match;
	}
}
