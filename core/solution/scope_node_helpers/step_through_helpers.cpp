#include "scope_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"

using namespace std;

void ScopeNode::step_through_activate(AbstractNode*& curr_node,
									  Problem* problem,
									  vector<ContextLayer>& context,
									  int& exit_depth,
									  AbstractNode*& exit_node,
									  RunHelper& run_helper,
									  ScopeNodeHistory* history) {
	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	history->scope_history = scope_history;
	context.back().scope_history = scope_history;

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->scope->step_through_activate(problem,
									   context,
									   inner_exit_depth,
									   inner_exit_node,
									   run_helper,
									   scope_history);

	history->throw_id = run_helper.throw_id;

	context.pop_back();

	context.back().node = NULL;

	if (run_helper.exceeded_limit
			|| run_helper.should_restart) {
		// do nothing
	} else if (run_helper.throw_id != -1) {
		string input_gate;
		cin >> input_gate;

		problem->print();
		cout << "ScopeNode" << endl;
		cout << "throw: " << run_helper.throw_id << endl;
		cout << "context:" << endl;
		context.back().node = this;
		for (int c_index = 0; c_index < (int)context.size(); c_index++) {
			cout << c_index << ": " << context[c_index].scope->id << " " << context[c_index].node->id << endl;
		}
		context.back().node = NULL;

		map<int, AbstractNode*>::iterator it = this->catches.find(run_helper.throw_id);
		if (it != this->catches.end()) {
			run_helper.throw_id = -1;

			curr_node = it->second;
		}
		// else do nothing
	} else if (inner_exit_depth == -1) {
		string input_gate;
		cin >> input_gate;

		problem->print();
		cout << "ScopeNode" << endl;
		cout << "exit" << endl;
		cout << "context:" << endl;
		context.back().node = this;
		for (int c_index = 0; c_index < (int)context.size(); c_index++) {
			cout << c_index << ": " << context[c_index].scope->id << " " << context[c_index].node->id << endl;
		}
		context.back().node = NULL;

		curr_node = this->next_node;
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}
