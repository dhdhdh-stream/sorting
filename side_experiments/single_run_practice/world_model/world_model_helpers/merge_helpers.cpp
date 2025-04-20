#include "world_model_helpers.h"

using namespace std;

bool attempt_merge(State* first,
				   State* second) {
	map<State*, State*> mapping;
	vector<pair<State*, State*>> next_pairs;
	next_pairs.push_back({first, second});

	while (next_pairs.size() > 0) {
		State* curr_first = next_pairs.back().first;
		State* curr_second = next_pairs.back().second;
		next_pairs.pop_back();

		

		if (abs(curr_first->obs_average - curr_second->obs_average)
				> EQUAL_STATE_AVERAGE_MAX_DIFF * world->obs_standard_deviation) {
			return false;
		}
		if (curr_first->obs_standard_deviation > EQUAL_STATE_STANDARD_DEVIATION_MAX_DIFF * curr_second->obs_standard_deviation
				|| curr_second->obs_standard_deviation > EQUAL_STATE_STANDARD_DEVIATION_MAX_DIFF * curr_first->obs_standard_deviation) {
			return false;
		}

		for (int c_index = 0; c_index < (int)curr_first->connections.size(); c_index++) {
			if (curr_first) {

			}
		}
	}
}
