#include "helpers.h"

#include <map>

#include "globals.h"
#include "try.h"
#include "try_impact.h"
#include "try_tracker.h"

using namespace std;

void compare_tries(Try* original,
				   Try* potential,
				   int& distance,
				   vector<pair<int, int>>& diffs) {
	int original_length = (int)original->sequence.size();
	int potential_length = (int)potential->sequence.size();

	vector<vector<int>> d(original_length+1, vector<int>(potential_length+1));
	vector<vector<vector<pair<int, int>>>> d_diffs(original_length+1, vector<vector<pair<int, int>>>(potential_length+1));

	d[0][0] = 0;
	for (int o_index = 1; o_index < original_length+1; o_index++) {
		d[o_index][0] = o_index;
		d_diffs[o_index][0] = d_diffs[o_index-1][0];
		d_diffs[o_index][0].push_back({TRY_REMOVE, o_index-1});
	}
	for (int p_index = 1; p_index < potential_length+1; p_index++) {
		d[0][p_index] = p_index;
		d_diffs[0][p_index] = d_diffs[0][p_index-1];
		d_diffs[0][p_index].push_back({TRY_INSERT, p_index-1});
	}

	uniform_int_distribution<int> distribution(0, 1);
	for (int o_index = 1; o_index < original_length+1; o_index++) {
		for (int p_index = 1; p_index < potential_length+1; p_index++) {
			if (original->sequence[o_index-1] == potential->sequence[p_index-1]) {
				d[o_index][p_index] = d[o_index-1][p_index-1];
				d_diffs[o_index][p_index] = d_diffs[o_index-1][p_index-1];
			} else {
				int insertion = d[o_index][p_index-1] + 1;
				int deletion = d[o_index-1][p_index] + 1;

				bool is_insert;
				if (insertion == deletion) {
					is_insert = distribution(generator) == 0;
				} else if (insertion < deletion) {
					is_insert = true;
				} else {
					is_insert = false;
				}

				if (is_insert) {
					d[o_index][p_index] = insertion;
					d_diffs[o_index][p_index] = d_diffs[o_index][p_index-1];
					d_diffs[o_index][p_index].push_back({TRY_INSERT, p_index-1});
				} else {
					d[o_index][p_index] = deletion;
					d_diffs[o_index][p_index] = d_diffs[o_index-1][p_index];
					d_diffs[o_index][p_index].push_back({TRY_REMOVE, o_index-1});
				}
			}
		}
	}

	distance = d.back().back();
	diffs = d_diffs.back().back();
}
