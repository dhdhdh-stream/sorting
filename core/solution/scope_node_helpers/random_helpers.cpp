#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "scope.h"

using namespace std;

void ScopeNode::random_existing_activate(AbstractNode*& curr_node,
										 vector<Scope*>& scope_context,
										 vector<AbstractNode*>& node_context,
										 int& exit_depth,
										 AbstractNode*& exit_node,
										 int& random_curr_depth,
										 bool& random_exceeded_limit,
										 vector<AbstractNode*>& possible_nodes) {
	possible_nodes.push_back(this);

	node_context.back() = this;

	scope_context.push_back(this->scope);
	node_context.push_back(NULL);

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->scope->inner_random_existing_activate(this->starting_node,
												scope_context,
												node_context,
												inner_exit_depth,
												inner_exit_node,
												random_curr_depth,
												random_exceeded_limit);

	scope_context.pop_back();
	node_context.pop_back();

	node_context.back() = NULL;

	if (random_exceeded_limit) {
		// do nothing
	} else if (inner_exit_depth == -1) {
		curr_node = this->next_node;
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}

void ScopeNode::inner_random_existing_activate(AbstractNode*& curr_node,
											   vector<Scope*>& scope_context,
											   vector<AbstractNode*>& node_context,
											   int& exit_depth,
											   AbstractNode*& exit_node,
											   int& random_curr_depth,
											   bool& random_exceeded_limit) {
	node_context.back() = this;

	scope_context.push_back(this->scope);
	node_context.push_back(NULL);

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->scope->inner_random_existing_activate(this->starting_node,
												scope_context,
												node_context,
												inner_exit_depth,
												inner_exit_node,
												random_curr_depth,
												random_exceeded_limit);

	scope_context.pop_back();
	node_context.pop_back();

	node_context.back() = NULL;

	if (random_exceeded_limit) {
		// do nothing
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
											random_exceeded_limit);

	scope_context.pop_back();
	node_context.pop_back();

	node_context.back() = NULL;

	if (random_exceeded_limit) {
		// do nothing
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
											random_exceeded_limit);

	scope_context.pop_back();
	node_context.pop_back();

	node_context.back() = NULL;

	if (random_exceeded_limit) {
		// do nothing
	} else if (inner_exit_depth == -1) {
		curr_node = this->next_node;
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}
