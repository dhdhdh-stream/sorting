#include "new_action_helpers.h"

#include "globals.h"
#include "new_action_tracker.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void setup_new_action(Solution* parent_solution) {
	parent_solution->new_action_tracker = new NewActionTracker();
	parent_solution->new_action_tracker->init(parent_solution);

	while (parent_solution->new_action_tracker->node_trackers.size() < NEW_ACTION_TRY_NODES) {
		vector<AbstractNode*> possible_nodes;
		parent_solution->current->random_exit_activate(
			parent_solution->current->nodes[0],
			possible_nodes);

		uniform_int_distribution<int> start_distribution(0, possible_nodes.size()-1);
		int start_index = start_distribution(generator);

		uniform_int_distribution<int> is_branch_distribution(0, 1);
		bool is_branch = is_branch_distribution(generator) == 0;

		bool existing_match = false;
		map<AbstractNode*, NewActionNodeTracker*>::iterator it =
			parent_solution->new_action_tracker->node_trackers.find(possible_nodes[start_index]);
		if (it != parent_solution->new_action_tracker->node_trackers.end()) {
			if (it->second->is_branch == is_branch) {
				existing_match = true;
			}
		}

		if (!existing_match) {
			possible_nodes.push_back(NULL);
			geometric_distribution<int> exit_distribution(0.33);
			int exit_index = start_index + 1 + exit_distribution(generator);
			if (exit_index > (int)possible_nodes.size()-1) {
				exit_index = (int)possible_nodes.size()-1;
			}

			parent_solution->new_action_tracker->node_trackers[possible_nodes[start_index]] =
				new NewActionNodeTracker(is_branch,
										 possible_nodes[exit_index]);
		}
	}
}
