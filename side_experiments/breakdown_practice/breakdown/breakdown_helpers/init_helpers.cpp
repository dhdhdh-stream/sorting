#include "breakdown_helpers.h"

using namespace std;

int init_breakdown_helper(vector<int>& solution,
						  vector<int>& lower_bounds,
						  vector<int>& upper_bounds,
						  int start_index,
						  int end_index) {
	vector<int> split_scores(solution.size(), 0);
	for (int a_index = start_index; a_index < end_index; a_index++) {
		if (lower_bounds[a_index] > start_index) {
			split_scores[lower_bounds[a_index]]++;
		}
	}
	for (int a_index = start_index; a_index < end_index; a_index++) {
		if (upper_bounds[a_index] < end_index-1) {
			split_scores[upper_bounds[a_index]]++;
		}
	}

	int best_index = start_index;
	int best_val = split_scores[start_index];
	for (int a_index = start_index+1; a_index < end_index; a_index++) {
		if (split_scores[a_index] > best_val) {
			best_index = a_index;
			best_val = split_scores[a_index];
		}
	}

	return best_index;
}

void init_breakdown(vector<int>& solution,
					vector<int>& lower_bounds,
					vector<int>& upper_bounds,
					vector<pair<int,int>>& splits) {
	splits.push_back({0, (int)solution.size()});

	while (true) {
		bool all_ind = true;
		for (int s_index = 0; s_index < (int)splits.size(); s_index++) {
			int start_index = splits[s_index].first;
			int end_index = splits[s_index].second;

			bool is_ind = true;
			for (int a_index = start_index; a_index < end_index; a_index++) {
				if (lower_bounds[a_index] != start_index
						|| upper_bounds[a_index] != end_index-1) {
					is_ind = false;
					break;
				}
			}

			if (!is_ind) {
				int split_index = init_breakdown_helper(
					solution,
					lower_bounds,
					upper_bounds,
					start_index,
					end_index);

				splits.erase(splits.begin() + s_index);

				splits.insert(splits.begin() + s_index, {split_index, end_index});
				splits.insert(splits.begin() + s_index, {start_index, split_index});

				for (int a_index = start_index; a_index < split_index; a_index++) {
					if (upper_bounds[a_index] > split_index-1) {
						upper_bounds[a_index] = split_index-1;
					}
				}
				for (int a_index = split_index; a_index < end_index; a_index++) {
					if (lower_bounds[a_index] < split_index) {
						lower_bounds[a_index] = split_index;
					}
				}

				all_ind = false;
				break;
			}
		}

		if (all_ind) {
			break;
		}
	}
}
