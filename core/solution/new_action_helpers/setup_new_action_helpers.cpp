#include "new_action_helpers.h"

#include "globals.h"
#include "new_action_tracker.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void setup_new_action() {
	solution->new_action_tracker = new NewActionTracker();

	while (solution->new_action_tracker->node_trackers.size() < NEW_ACTION_TRY_NODES) {
		vector<AbstractNode*> possible_nodes;
		solution->current->random_exit_activate(
			solution->current->nodes[0],
			possible_nodes);

		uniform_int_distribution<int> start_distribution(0, possible_nodes.size()-1);
		int start_index = start_distribution(generator);

		map<AbstractNode*, NewActionNodeTracker*>::iterator it =
			solution->new_action_tracker->node_trackers.find(possible_nodes[start_index]);
		if (it == solution->new_action_tracker->node_trackers.end()) {
			possible_nodes.push_back(NULL);
			uniform_int_distribution<int> exit_distribution(start_index+1, possible_nodes.size()-1);
			int exit_index = exit_distribution(generator);

			solution->new_action_tracker->node_trackers[possible_nodes[start_index]] =
				new NewActionNodeTracker(possible_nodes[exit_index]);
		}
	}
}
