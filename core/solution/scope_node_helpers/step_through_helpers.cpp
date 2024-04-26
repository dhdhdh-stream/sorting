#include "scope_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"

using namespace std;

void ScopeNode::step_through_activate(AbstractNode*& curr_node,
									  Problem* problem,
									  vector<ContextLayer>& context,
									  RunHelper& run_helper,
									  ScopeNodeHistory* history) {
	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	history->scope_history = scope_history;
	context.back().scope_history = scope_history;

	this->scope->step_through_activate(problem,
									   context,
									   run_helper,
									   scope_history);

	context.pop_back();

	context.back().node = NULL;

	string input_gate;
	cin >> input_gate;

	cout << "context:" << endl;
	context.back().node = this;
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		cout << c_index << ": " << context[c_index].scope->id << " " << context[c_index].node->id << endl;
	}
	context.back().node = NULL;
	problem->print();
	cout << "ScopeNode" << endl;
	cout << "exit" << endl;

	curr_node = this->next_node;
}
