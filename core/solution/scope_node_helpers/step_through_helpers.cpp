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

	this->scope->step_through_activate(problem,
									   context,
									   run_helper);

	context.back().node = NULL;

	history->obs_snapshot = problem->get_observations();

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
