#include "world_model_helpers.h"

using namespace std;

bool check_is_complete(RunInstance* run_instance) {
	if (run_instance->curr_state != NULL) {
		set<State*> processed_states;
		set<State*> next_states;
		next_states.insert(run_instance->curr_state);

		while (next_states.size() > 0) {
			State* curr_state = *next_states.begin();

			for (int c_index = 0; c_index < (int)curr_state->connections.size(); c_index++) {
				if (curr_state->connections[c_index] == NULL) {
					return false;
				}
			}

			for (int c_index = 0; c_index < (int)curr_state->connections.size(); c_index++) {
				if (seen_states.find(curr_state) == seen_states.end()
						&& next_states.find(curr_state) == next_states.end()) {
					next_states.insert(curr_state->connections[c_index]);
				}
			}

			next_states.erase(curr_state);
			processed_states.insert(curr_state);
		}

		return true;
	} else {
		return false;
	}
}
