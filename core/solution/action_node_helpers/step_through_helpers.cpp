#include "action_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ActionNode::step_through_activate(AbstractNode*& curr_node,
									   Problem* problem,
									   vector<ContextLayer>& context,
									   RunHelper& run_helper,
									   ActionNodeHistory* history) {
	problem->perform_action(this->action);
	history->obs_snapshot = problem->get_observations();

	if (this->action.move != ACTION_NOOP) {
		string input_gate;
		cin >> input_gate;

		cout << "context:" << endl;
		context.back().node = this;
		for (int c_index = 0; c_index < (int)context.size(); c_index++) {
			cout << c_index << ": " << context[c_index].scope->id << " " << context[c_index].node->id << endl;
		}
		context.back().node = NULL;
		problem->print();
		cout << "ActionNode" << endl;
		cout << "this->action.move: " << this->action.move << endl;
	}

	curr_node = this->next_node;
}
