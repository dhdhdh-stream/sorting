#include "world_model_helpers.h"

using namespace std;

/**
 * - if connected, then fully connected
 *   - due to the way connections are created
 */
void gather_connected(RunInstance* run_instance,
					  set<State*>& connected_states) {
	vector<State*> next_states;
	while (next_states.size() > 0) {
		State* curr_state = next_states.back();
		next_states.pop_back();
		if (connected_states.find(curr_state) == connected_states.end()) {
			for (int c_index = 0; c_index < (int)curr_state->connections.size(); c_index++) {
				if (curr_state->connections[c_index] == NULL) {
					next_states.push_back(curr_state->connections[c_index]);
				}
			}
		}
	}
}
