#include "travel_helpers.h"

#include <cmath>

#include "constants.h"
#include "world_model.h"
#include "world_state.h"

using namespace std;

void build_travel_map(WorldModel* world_model) {
	world_model->travel_map = vector<vector<int>>(world_model->states.size());
	for (int starting_index = 0; starting_index < (int)world_model->states.size(); starting_index++) {
		vector<bool> states_traveled_from(world_model->states.size(), false);
		vector<double> max_likelihood_to(world_model->states.size(), 0.0);
		vector<int> starting_action(world_model->states.size(), -1);

		for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
			for (int t_index = 0; t_index < (int)world_model->states[starting_index]->transitions[a_index].size(); t_index++) {
				int next_state_index = world_model->states[starting_index]->transitions[a_index][t_index].first;
				double likelihood = log(world_model->states[starting_index]->transitions[a_index][t_index].second);
				if (likelihood > max_likelihood_to[next_state_index]) {
					max_likelihood_to[next_state_index] = likelihood;
					starting_action[next_state_index] = a_index;
				}
			}
		}
		states_traveled_from[starting_index] = true;

		while (true) {
			int highest_state_index = -1;
			double highest_likelihood = 0.0;
			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				if (!states_traveled_from[s_index]
						&& max_likelihood_to[s_index] > highest_likelihood) {
					highest_state_index = s_index;
					highest_likelihood = max_likelihood_to[s_index];
				}
			}

			if (highest_state_index == -1) {
				break;
			}

			WorldState* state = world_model->states[highest_state_index];
			double state_likelihood = max_likelihood_to[highest_state_index];
			for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
				for (int t_index = 0; t_index < (int)state->transitions[a_index].size(); t_index++) {
					int next_state_index = state->transitions[a_index][t_index].first;
					double likelihood = state_likelihood + log(state->transitions[a_index][t_index].second);
					if (likelihood > max_likelihood_to[next_state_index]) {
						max_likelihood_to[next_state_index] = likelihood;
						starting_action[next_state_index] = starting_action[highest_state_index];
					}
				}
			}
			states_traveled_from[highest_state_index] = true;
		}

		world_model->travel_map[starting_index] = starting_action;
	}
}
