#include "segment_helpers.h"

#include <cmath>
#include <iostream>

#include "globals.h"
#include "problem.h"
#include "sample.h"

using namespace std;

double calc_distance(Sample* sample,
					 int starting_index,
					 int ending_index) {
	if (ending_index - starting_index <= 1) {
		return 0.0;
	}

	vector<double> step(problem_type->num_dimensions());
	for (int d_index = 0; d_index < problem_type->num_dimensions(); d_index++) {
		step[d_index] = ((double)(sample->locations[ending_index][0][d_index]
				- sample->locations[starting_index][0][d_index]))
			/ ((double)(ending_index - starting_index));
	}

	double sum_distance = 0.0;
	for (int i_index = starting_index + 1; i_index < ending_index; i_index++) {
		double distance = 0.0;
		for (int d_index = 0; d_index < problem_type->num_dimensions(); d_index++) {
			double target = sample->locations[starting_index][0][d_index]
				+ (i_index - starting_index) * step[d_index];

			distance += (sample->locations[i_index][0][d_index] - target)
				* (sample->locations[i_index][0][d_index] - target);
		}

		sum_distance += sqrt(distance);
	}

	return sum_distance;
}

double calc_distance(Sample* sample,
					 vector<int>& starting_indexes,
					 vector<int>& ending_indexes) {
	double sum_distance = 0.0;
	for (int s_index = 0; s_index < (int)starting_indexes.size(); s_index++) {
		sum_distance += calc_distance(sample,
									  starting_indexes[s_index],
									  ending_indexes[s_index]);
	}

	return sum_distance;
}

void find_max_split(Sample* sample,
					vector<int>& starting_indexes,
					vector<int>& ending_indexes) {
	vector<double> existing_distances(starting_indexes.size());
	for (int s_index = 0; s_index < (int)starting_indexes.size(); s_index++) {
		existing_distances[s_index] = calc_distance(sample,
													starting_indexes[s_index],
													ending_indexes[s_index]);
	}

	int best_segment_index = -1;
	int best_split_index = -1;
	double best_improvement = 0.0;
	for (int s_index = 0; s_index < (int)starting_indexes.size(); s_index++) {
		for (int i_index = starting_indexes[s_index] + 1; i_index <= ending_indexes[s_index]; i_index++) {
			double front_distance = calc_distance(sample,
												  starting_indexes[s_index],
												  i_index-1);
			double back_distance = calc_distance(sample,
												 i_index,
												 ending_indexes[s_index]);
			double improvement = existing_distances[s_index] - (front_distance + back_distance);
			if (improvement > best_improvement) {
				best_segment_index = s_index;
				best_split_index = i_index;
				best_improvement = improvement;
			}
		}
	}

	vector<int> next_starting_indexes;
	vector<int> next_ending_indexes;
	for (int s_index = 0; s_index < (int)starting_indexes.size(); s_index++) {
		if (s_index == best_segment_index) {
			next_starting_indexes.push_back(starting_indexes[s_index]);
			next_ending_indexes.push_back(best_split_index-1);

			next_starting_indexes.push_back(best_split_index);
			next_ending_indexes.push_back(ending_indexes[s_index]);
		} else {
			next_starting_indexes.push_back(starting_indexes[s_index]);
			next_ending_indexes.push_back(ending_indexes[s_index]);
		}
	}

	starting_indexes = next_starting_indexes;
	ending_indexes = next_ending_indexes;
}
