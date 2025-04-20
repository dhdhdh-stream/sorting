#include "world_model_helpers.h"

using namespace std;

void travel(Problem* problem,
			RunInstance* run_instance,
			State* target_location) {
	map<State*, pair<int, State*>> seen_states;
	seen_states[run_instance->curr_state] = {-1, NULL};
	list<State*> next_states;

	bool is_done = false;
	while (!is_done) {
		State* curr_state = next_states.front();
		next_states.pop_front();

		if (seen_states.find(curr_state) == seen_states.end()) {
			for (int c_index = 0; c_index < (int)curr_state->connections.size(); c_index++) {
				if (seen_states.find(curr_state->connections[c_index]) == seen_states.end()) {
					seen_states[curr_state->connections[c_index]] = {c_index, curr_state};
					next_states.push_back(curr_state->connections[c_index]);

					if (curr_state->connections[c_index] == target_location) {
						is_done = true;
					}
				}
			}
		}
	}

	vector<int> action_sequence;
	State* curr_state = target_location;
	while (true) {
		map<State*, pair<int, State*>>::iterator it = seen_states.find(curr_state);
		if (it->second.second == NULL) {
			break;
		} else {
			action_sequence.push_back(it->second.first);
			curr_state = it->second.second;
		}
	}

	for (int a_index = (int)action_sequence.size()-1; a_index >= 0; a_index--) {
		problem->perform_action(Action(action_sequence[a_index]));
	}
	run_instance.curr_state = target_location;
}

void get_random_unknown(RunInstance* run_instance,
						State*& random_state,
						Action& random_action) {
	vector<pair<State*, Action>> possibilities;

	set<State*> processed_states;
	set<State*> next_states;
	next_states.insert(run_instance->curr_state);

	while (next_states.size() > 0) {
		State* curr_state = *next_states.begin();

		for (int c_index = 0; c_index < (int)curr_state->connections.size(); c_index++) {
			if (curr_state->connections[c_index] == NULL) {
				possibilities.push_back({curr_state, Action(c_index)});
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

	uniform_int_distribution<int> distribution(0, possibilities.size()-1);
	int random_index = distribution(generator);
	random_state = possibilities[random_index].first;
	random_action = possibilities[random_index].second;
}
