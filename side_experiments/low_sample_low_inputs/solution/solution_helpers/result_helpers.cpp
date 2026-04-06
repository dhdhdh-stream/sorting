#include "solution_helpers.h"

#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

double get_existing_result(SolutionWrapper* wrapper) {
	Problem* copy_problem = wrapper->problem->copy_and_reset();

	vector<ScopeHistory*> scope_histories;
	vector<AbstractNode*> node_context;
	int num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(wrapper->solution->scopes[0]);
	scope_histories.push_back(scope_history);
	node_context.push_back(wrapper->solution->scopes[0]->nodes[0]);

	while (true) {
		vector<double> obs = copy_problem->get_observations();

		int action;
		bool is_next = false;
		bool is_done = false;
		while (!is_next) {
			if (node_context.back() == NULL) {
				if (scope_histories.size() == 1) {
					is_next = true;
					is_done = true;
				} else {
					ScopeNode* scope_node = (ScopeNode*)node_context[node_context.size() - 2];
					scope_node->exit_step(scope_histories,
										  node_context);
				}
			} else {
				node_context.back()->step(obs,
										  action,
										  is_next,
										  scope_histories,
										  node_context,
										  num_actions);
			}
		}

		if (is_done) {
			break;
		} else {
			copy_problem->perform_action(action);
		}
	}

	double target_val = copy_problem->score_result();
	target_val -= 0.0001 * num_actions;

	delete scope_histories[0];

	delete copy_problem;

	return target_val;
}
