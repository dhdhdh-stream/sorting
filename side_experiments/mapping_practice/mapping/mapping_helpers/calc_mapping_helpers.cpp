/**
 * - mapping not Markov, so solving optimally expensive
 */

#include "mapping_helpers.h"

#include "world.h"

using namespace std;

void calc_mapping_helper(vector<int>& obs,
						 Assignment& assignment,
						 Mapping& mapping) {
	mapping.curr_x = assignment.init_x;
	mapping.curr_y = assignment.init_y;

	mapping.world[mapping.curr_x][mapping.curr_y][obs[0]] = 1;

	for (int i_index = 0; i_index < (int)assignment.x_impact.size(); i_index++) {
		mapping.curr_x += assignment.x_impact[i_index];
		if (mapping.curr_x < 0) {
			mapping.curr_x = 0;
		} else if (mapping.curr_x >= WORLD_WIDTH) {
			mapping.curr_x = WORLD_WIDTH-1;
		}
		mapping.curr_y += assignment.y_impact[i_index];
		if (mapping.curr_y < 0) {
			mapping.curr_y = 0;
		} else if (mapping.curr_y >= WORLD_HEIGHT) {
			mapping.curr_y = WORLD_HEIGHT-1;
		}

		map<int,int>::iterator it = mapping.world[mapping.curr_x][mapping.curr_y].find(obs[1 + i_index]);
		if (it == mapping.world[mapping.curr_x][mapping.curr_y].end()) {
			mapping.world[mapping.curr_x][mapping.curr_y][obs[1 + i_index]] = 1;
		} else {
			it->second++;
		}
	}
}

double calc_conflict_helper(Mapping& mapping) {
	double sum_conflict = 0.0;
	for (int x_index = 0; x_index < WORLD_WIDTH; x_index++) {
		for (int y_index = 0; y_index < WORLD_HEIGHT; y_index++) {
			if (mapping.world[x_index][y_index].size() > 1) {
				int sum_count = 0;
				for (map<int,int>::iterator it = mapping.world[x_index][y_index].begin();
						it != mapping.world[x_index][y_index].end(); it++) {
					sum_count += it->second;
				}

				for (map<int,int>::iterator it = mapping.world[x_index][y_index].begin();
						it != mapping.world[x_index][y_index].end(); it++) {
					double ratio = (double)it->second / (double)sum_count;
					sum_conflict += it->second * (1.0 - ratio);
				}
			}
		}
	}

	return sum_conflict;
}
