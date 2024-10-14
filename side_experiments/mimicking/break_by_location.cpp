// - issues with trying to mimic:
//   - there may be no alignment
//   - solution too complex to learn off few samples
//     - if need A and B and C before starts paying off, then difficult to start with
//   - solution too comples to have common sequences

// - maybe mimicking just not that feasible
//   - solutions too complicated/varied to be mimicked directly
//   - more about teaching/learning
//     - teach/learn building blocks, and use to build up to larger solutions
//       - build shared vocabulary (rather than alignment)

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "sample.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;

const int MIN_SEGMENT_SIZE = 6;

const int MAX_LOCATION_DISTANCE = 3;

string print_helper(int move) {
	switch (move) {
	case ACTION_NOOP:
		return "NOOP";
	case MINESWEEPER_ACTION_UP:
		return "UP";
	case MINESWEEPER_ACTION_RIGHT:
		return "RIGHT";
	case MINESWEEPER_ACTION_DOWN:
		return "DOWN";
	case MINESWEEPER_ACTION_LEFT:
		return "LEFT";
	case MINESWEEPER_ACTION_CLICK:
		return "CLICK";
	case MINESWEEPER_ACTION_FLAG:
		return "FLAG";
	case MINESWEEPER_ACTION_DOUBLECLICK:
		return "DOUBLECLICK";
	}

	return "";
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	Sample sample(1);

	vector<pair<pair<int,int>,bool>> segments{{{1, sample.actions.size()}, false}};
	while (true) {
		int max_size = 0;
		int max_segment_index = -1;
		int max_start_index = -1;
		int max_end_index = -1;
		for (int s_index = 0; s_index < (int)segments.size(); s_index++) {
			if (!segments[s_index].second
					&& segments[s_index].first.second - segments[s_index].first.first > MIN_SEGMENT_SIZE) {
				for (int l_index = segments[s_index].first.first; l_index < segments[s_index].first.second - MIN_SEGMENT_SIZE; l_index++) {
					int max_x = sample.locations[l_index-1][0][0];
					int min_x = sample.locations[l_index-1][0][0];
					int max_y = sample.locations[l_index-1][0][1];
					int min_y = sample.locations[l_index-1][0][1];

					int curr_index = l_index + 1;
					while (true) {
						if (sample.locations[curr_index-1][0][0] > max_x) {
							max_x = sample.locations[curr_index-1][0][0];
						}
						if (sample.locations[curr_index-1][0][0] < min_x) {
							min_x = sample.locations[curr_index-1][0][0];
						}
						if (sample.locations[curr_index-1][0][1] > max_y) {
							max_y = sample.locations[curr_index-1][0][1];
						}
						if (sample.locations[curr_index-1][0][1] < min_y) {
							min_y = sample.locations[curr_index-1][0][1];
						}

						if (max_x - min_x >= MAX_LOCATION_DISTANCE
								|| max_y - min_y >= MAX_LOCATION_DISTANCE) {
							break;
						}

						curr_index++;
						if (curr_index >= segments[s_index].first.second) {
							break;
						}
					}

					int curr_distance = curr_index - l_index;
					if (curr_distance > max_size) {
						max_size = curr_distance;
						max_segment_index = s_index;
						max_start_index = l_index;
						max_end_index = curr_index;
					}
				}
			}
		}

		bool should_continue = false;
		if (max_size > MIN_SEGMENT_SIZE) {
			int existing_start = segments[max_segment_index].first.first;
			int existing_end = segments[max_segment_index].first.second;

			segments.erase(segments.begin() + max_segment_index);

			if (max_end_index < existing_end) {
				segments.insert(segments.begin() + max_segment_index, {{max_end_index, existing_end}, false});
			}
			segments.insert(segments.begin() + max_segment_index, {{max_start_index, max_end_index}, true});
			if (max_start_index > existing_start) {
				segments.insert(segments.begin() + max_segment_index, {{existing_start, max_start_index}, false});
			}

			should_continue = true;
		}
		if (!should_continue) {
			break;
		}
	}

	for (int s_index = 0; s_index < (int)segments.size(); s_index++) {
		if (segments[s_index].second) {
			for (int l_index = segments[s_index].first.first; l_index < segments[s_index].first.second; l_index++) {
				cout << print_helper(sample.actions[l_index].move) << " ";
			}
			cout << endl;
		} else {
			for (int l_index = segments[s_index].first.first; l_index < segments[s_index].first.second; l_index++) {
				cout << print_helper(sample.actions[l_index].move) << endl;
			}
		}
	}

	delete problem_type;

	cout << "Done" << endl;
}
