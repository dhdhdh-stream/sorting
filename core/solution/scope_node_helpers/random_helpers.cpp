#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "scope.h"

using namespace std;

void ScopeNode::random_activate(AbstractNode*& curr_node,
								vector<Scope*>& scope_context,
								vector<AbstractNode*>& node_context,
								int& exit_depth,
								AbstractNode*& exit_node,
								int& random_curr_depth,
								int& random_throw_id,
								bool& random_exceeded_limit,
								vector<vector<Scope*>>& possible_scope_contexts,
								vector<vector<AbstractNode*>>& possible_node_contexts) {
	node_context.back() = this;

	scope_context.push_back(this->scope);
	node_context.push_back(NULL);

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	vector<vector<Scope*>> inner_possible_scope_contexts;
	vector<vector<AbstractNode*>> inner_possible_node_contexts;
	this->scope->random_activate(this->starting_node,
								 scope_context,
								 node_context,
								 inner_exit_depth,
								 inner_exit_node,
								 random_curr_depth,
								 random_throw_id,
								 random_exceeded_limit,
								 inner_possible_scope_contexts,
								 inner_possible_node_contexts);

	scope_context.pop_back();
	node_context.pop_back();

	uniform_int_distribution<int> distribution(0, 2);
	if (distribution(generator) != 0) {
		possible_scope_contexts.insert(possible_scope_contexts.begin(),
			inner_possible_scope_contexts.begin(),
			inner_possible_scope_contexts.end());
		possible_node_contexts.insert(possible_node_contexts.begin(),
			inner_possible_node_contexts.begin(),
			inner_possible_node_contexts.end());
	} else {
		possible_scope_contexts.push_back(scope_context);
		possible_node_contexts.push_back(node_context);
	}

	node_context.back() = NULL;

	if (random_exceeded_limit) {
		// do nothing
	} else if (random_throw_id != -1) {
		map<int, AbstractNode*>::iterator it = this->catches.find(random_throw_id);
		if (it != this->catches.end()) {
			random_throw_id = -1;
			curr_node = it->second;
		}
		// else do nothing
	} else if (inner_exit_depth == -1) {
		curr_node = this->next_node;
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}

void ScopeNode::random_exit_activate(AbstractNode*& curr_node,
									 vector<Scope*>& scope_context,
									 vector<AbstractNode*>& node_context,
									 int& exit_depth,
									 AbstractNode*& exit_node,
									 int& random_curr_depth,
									 int& random_throw_id,
									 bool& random_exceeded_limit,
									 int curr_depth,
									 vector<pair<int,AbstractNode*>>& possible_exits) {
	node_context.back() = this;

	possible_exits.push_back({curr_depth, curr_node});

	scope_context.push_back(this->scope);
	node_context.push_back(NULL);

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->scope->inner_random_exit_activate(this->starting_node,
											scope_context,
											node_context,
											inner_exit_depth,
											inner_exit_node,
											random_curr_depth,
											random_throw_id,
											random_exceeded_limit);

	scope_context.pop_back();
	node_context.pop_back();

	node_context.back() = NULL;

	if (random_exceeded_limit) {
		// do nothing
	} else if (random_throw_id != -1) {
		map<int, AbstractNode*>::iterator it = this->catches.find(random_throw_id);
		if (it != this->catches.end()) {
			random_throw_id = -1;
			curr_node = it->second;
		}
		// else do nothing
	} else if (inner_exit_depth == -1) {
		curr_node = this->next_node;
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}

void ScopeNode::inner_random_exit_activate(AbstractNode*& curr_node,
										   vector<Scope*>& scope_context,
										   vector<AbstractNode*>& node_context,
										   int& exit_depth,
										   AbstractNode*& exit_node,
										   int& random_curr_depth,
										   int& random_throw_id,
										   bool& random_exceeded_limit) {
	node_context.back() = this;

	scope_context.push_back(this->scope);
	node_context.push_back(NULL);

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->scope->inner_random_exit_activate(this->starting_node,
											scope_context,
											node_context,
											inner_exit_depth,
											inner_exit_node,
											random_curr_depth,
											random_throw_id,
											random_exceeded_limit);

	scope_context.pop_back();
	node_context.pop_back();

	node_context.back() = NULL;

	if (random_exceeded_limit) {
		// do nothing
	} else if (random_throw_id != -1) {
		map<int, AbstractNode*>::iterator it = this->catches.find(random_throw_id);
		if (it != this->catches.end()) {
			random_throw_id = -1;
			curr_node = it->second;
		}
		// else do nothing
	} else if (inner_exit_depth == -1) {
		curr_node = this->next_node;
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}
