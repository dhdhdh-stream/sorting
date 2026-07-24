#include "solution_helpers.h"

#include <iostream>

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

	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
			curr_context.push_back(h_it->first);
			curr_index.push_back(h_it->second->index);
			gather_dependencies_helper(scope_node_history->scope_history,
									   curr_context,
									   curr_index,
									   count,
									   dependency,
									   index);
			curr_context.pop_back();
			curr_index.pop_back();
		}

		uniform_int_distribution<int> distribution(0, count);
		count++;
		if (distribution(generator) == 0) {
			curr_context.push_back(h_it->first);
			curr_index.push_back(h_it->second->index);
			dependency = curr_context;
			index = curr_index;
			curr_context.pop_back();
			curr_index.pop_back();
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

	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->index <= top_index) {
			if (h_it->second->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				curr_context.push_back(h_it->first);
				curr_index.push_back(h_it->second->index);
				gather_dependencies_helper(scope_node_history->scope_history,
										   curr_context,
										   curr_index,
										   count,
										   dependency,
										   index);
				curr_context.pop_back();
				curr_index.pop_back();
			}

			uniform_int_distribution<int> distribution(0, count);
			count++;
			if (distribution(generator) == 0) {
				curr_context.push_back(h_it->first);
				curr_index.push_back(h_it->second->index);
				dependency = curr_context;
				index = curr_index;
				curr_context.pop_back();
				curr_index.pop_back();
			}
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
			scope->nodes[dependency[l_index]]->dependencies.push_back(experiment);
		}
	} else {
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[dependency[l_index]];
		set_dependency_helper(scope_node->scope,
							  dependency,
							  l_index+1,
							  experiment);
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
			AbstractNode* node = scope->nodes[dependency[l_index]];
			for (int d_index = 0; d_index < (int)node->dependencies.size(); d_index++) {
				if (node->dependencies[d_index] == experiment) {
					node->dependencies.erase(node->dependencies.begin() + d_index);
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
							 int& num_actions,
							 Eigen::VectorXf& state_norms,
							 Eigen::VectorXf& state,
							 vector<double>& obs) {
	if (dependency[l_index] == -1) {
		is_hit = true;
		num_actions = scope_history->num_actions;
		state_norms = scope_history->state_norms;
		state = scope_history->state;
		obs = scope_history->obs;
	} else {
		map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(dependency[l_index]);
		if (it == scope_history->node_histories.end()) {
			is_hit = false;
		} else {
			if (l_index == (int)dependency.size()-1) {
				is_hit = true;
				num_actions = it->second->num_actions;
				state_norms = it->second->state_norms;
				state = it->second->state;
				obs = it->second->obs;
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				fetch_dependency_helper(scope_node_history->scope_history,
										dependency,
										l_index+1,
										is_hit,
										num_actions,
										state_norms,
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
			scope->nodes[dependency[l_index]]->init_network_scope_contexts.push_back(init_network_scope_context);
			scope->nodes[dependency[l_index]]->init_network_node_contexts.push_back(dependency);
			scope->nodes[dependency[l_index]]->init_networks.push_back(init_network);
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
