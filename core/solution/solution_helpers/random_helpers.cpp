#include "solution_helpers.h"

#include "context_layer.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void random_sequence(AbstractNode*& curr_node,
					 Problem* problem,
					 vector<ContextLayer>& context,
					 RunHelper& run_helper) {
	solution->num_actions_until_random = -1;

	uniform_int_distribution<int> uniform_distribution(0, 1);
	geometric_distribution<int> geometric_distribution(0.5);
	int new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);

	uniform_int_distribution<int> default_distribution(0, 3);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		bool default_to_action = true;
		if (default_distribution(generator) != 0) {
			ScopeNode* new_scope_node = create_existing();
			if (new_scope_node != NULL) {
				new_scope_node->explore_activate(
					problem,
					context,
					run_helper);
				delete new_scope_node;

				default_to_action = false;
			}
		}

		if (default_to_action) {
			problem->perform_action(problem_type->random_action());
		}
	}

	vector<AbstractNode*> possible_exits;
	context.back().scope->random_exit_activate(
		curr_node,
		possible_exits);
	uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
	curr_node = possible_exits[distribution(generator)];

	uniform_int_distribution<int> next_distribution(0, (int)(2.0 * solution->average_num_actions));
	solution->num_actions_until_random = 1 + next_distribution(generator);
}