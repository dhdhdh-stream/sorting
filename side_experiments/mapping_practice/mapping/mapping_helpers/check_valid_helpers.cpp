/**
 * - once mapping assigned, becomes Markov
 */

#include "mapping_helpers.h"

#include <iostream>
#include <set>

#include "world.h"

using namespace std;

bool check_valid(vector<int>& obs,
				 vector<int>& actions,
				 vector<vector<int>>& mapping) {
	int curr_index = 0;
	set<pair<int,int>> possibilities;

	for (int x_index = 0; x_index < WORLD_WIDTH; x_index++) {
		for (int y_index = 0; y_index < WORLD_HEIGHT; y_index++) {
			if (mapping[x_index][y_index] == obs[0]) {
				possibilities.insert({x_index, y_index});
			}
		}
	}

	while (true) {
		if (curr_index >= (int)actions.size()) {
			break;
		}
		if (possibilities.size() == 0) {
			break;
		}

		set<pair<int,int>> next_possibilities;
		for (set<pair<int,int>>::iterator it = possibilities.begin();
				it != possibilities.end(); it++) {
			switch (actions[curr_index]) {
			case ACTION_UP:
				for (int x_diff = -1; x_diff <= 1; x_diff++) {
					for (int y_diff = 1; y_diff <= 2; y_diff++) {
						int next_x = it->first + x_diff;
						if (next_x < 0) {
							next_x = 0;
						} else if (next_x >= WORLD_WIDTH) {
							next_x = WORLD_WIDTH-1;
						}
						int next_y = it->second + y_diff;
						if (next_y < 0) {
							next_y = 0;
						} else if (next_y >= WORLD_HEIGHT) {
							next_y = WORLD_HEIGHT-1;
						}

						if (mapping[next_x][next_y] == obs[1 + curr_index]) {
							next_possibilities.insert({next_x, next_y});
						}
					}
				}
				break;
			case ACTION_RIGHT:
				for (int x_diff = 1; x_diff <= 2; x_diff++) {
					for (int y_diff = -1; y_diff <= 1; y_diff++) {
						int next_x = it->first + x_diff;
						if (next_x < 0) {
							next_x = 0;
						} else if (next_x >= WORLD_WIDTH) {
							next_x = WORLD_WIDTH-1;
						}
						int next_y = it->second + y_diff;
						if (next_y < 0) {
							next_y = 0;
						} else if (next_y >= WORLD_HEIGHT) {
							next_y = WORLD_HEIGHT-1;
						}

						if (mapping[next_x][next_y] == obs[1 + curr_index]) {
							next_possibilities.insert({next_x, next_y});
						}
					}
				}
				break;
			case ACTION_DOWN:
				for (int x_diff = -1; x_diff <= 1; x_diff++) {
					for (int y_diff = -2; y_diff <= -1; y_diff++) {
						int next_x = it->first + x_diff;
						if (next_x < 0) {
							next_x = 0;
						} else if (next_x >= WORLD_WIDTH) {
							next_x = WORLD_WIDTH-1;
						}
						int next_y = it->second + y_diff;
						if (next_y < 0) {
							next_y = 0;
						} else if (next_y >= WORLD_HEIGHT) {
							next_y = WORLD_HEIGHT-1;
						}

						if (mapping[next_x][next_y] == obs[1 + curr_index]) {
							next_possibilities.insert({next_x, next_y});
						}
					}
				}
				break;
			case ACTION_LEFT:
				for (int x_diff = -2; x_diff <= -1; x_diff++) {
					for (int y_diff = -1; y_diff <= 1; y_diff++) {
						int next_x = it->first + x_diff;
						if (next_x < 0) {
							next_x = 0;
						} else if (next_x >= WORLD_WIDTH) {
							next_x = WORLD_WIDTH-1;
						}
						int next_y = it->second + y_diff;
						if (next_y < 0) {
							next_y = 0;
						} else if (next_y >= WORLD_HEIGHT) {
							next_y = WORLD_HEIGHT-1;
						}

						if (mapping[next_x][next_y] == obs[1 + curr_index]) {
							next_possibilities.insert({next_x, next_y});
						}
					}
				}
				break;
			}
		}

		curr_index++;
		possibilities = next_possibilities;
	}

	if (curr_index < (int)actions.size()) {
		return false;
	} else {
		return true;
	}
}

bool check_valid(vector<int>& obs,
				 vector<int>& actions,
				 vector<vector<int>>& map_vals,
				 vector<vector<bool>>& map_assigned) {
	int curr_index = 0;
	set<pair<int,int>> possibilities;

	for (int x_index = 0; x_index < WORLD_WIDTH; x_index++) {
		for (int y_index = 0; y_index < WORLD_HEIGHT; y_index++) {
			if (!map_assigned[x_index][y_index]
					|| map_vals[x_index][y_index] == obs[0]) {
				possibilities.insert({x_index, y_index});
			}
		}
	}

	while (true) {
		if (curr_index >= (int)actions.size()) {
			break;
		}
		if (possibilities.size() == 0) {
			break;
		}

		set<pair<int,int>> next_possibilities;
		for (set<pair<int,int>>::iterator it = possibilities.begin();
				it != possibilities.end(); it++) {
			switch (actions[curr_index]) {
			case ACTION_UP:
				for (int x_diff = -1; x_diff <= 1; x_diff++) {
					for (int y_diff = 1; y_diff <= 2; y_diff++) {
						int next_x = it->first + x_diff;
						if (next_x < 0) {
							next_x = 0;
						} else if (next_x >= WORLD_WIDTH) {
							next_x = WORLD_WIDTH-1;
						}
						int next_y = it->second + y_diff;
						if (next_y < 0) {
							next_y = 0;
						} else if (next_y >= WORLD_HEIGHT) {
							next_y = WORLD_HEIGHT-1;
						}

						if (!map_assigned[next_x][next_y]
								|| map_vals[next_x][next_y] == obs[1 + curr_index]) {
							next_possibilities.insert({next_x, next_y});
						}
					}
				}
				break;
			case ACTION_RIGHT:
				for (int x_diff = 1; x_diff <= 2; x_diff++) {
					for (int y_diff = -1; y_diff <= 1; y_diff++) {
						int next_x = it->first + x_diff;
						if (next_x < 0) {
							next_x = 0;
						} else if (next_x >= WORLD_WIDTH) {
							next_x = WORLD_WIDTH-1;
						}
						int next_y = it->second + y_diff;
						if (next_y < 0) {
							next_y = 0;
						} else if (next_y >= WORLD_HEIGHT) {
							next_y = WORLD_HEIGHT-1;
						}

						if (!map_assigned[next_x][next_y]
								|| map_vals[next_x][next_y] == obs[1 + curr_index]) {
							next_possibilities.insert({next_x, next_y});
						}
					}
				}
				break;
			case ACTION_DOWN:
				for (int x_diff = -1; x_diff <= 1; x_diff++) {
					for (int y_diff = -2; y_diff <= -1; y_diff++) {
						int next_x = it->first + x_diff;
						if (next_x < 0) {
							next_x = 0;
						} else if (next_x >= WORLD_WIDTH) {
							next_x = WORLD_WIDTH-1;
						}
						int next_y = it->second + y_diff;
						if (next_y < 0) {
							next_y = 0;
						} else if (next_y >= WORLD_HEIGHT) {
							next_y = WORLD_HEIGHT-1;
						}

						if (!map_assigned[next_x][next_y]
								|| map_vals[next_x][next_y] == obs[1 + curr_index]) {
							next_possibilities.insert({next_x, next_y});
						}
					}
				}
				break;
			case ACTION_LEFT:
				for (int x_diff = -2; x_diff <= -1; x_diff++) {
					for (int y_diff = -1; y_diff <= 1; y_diff++) {
						int next_x = it->first + x_diff;
						if (next_x < 0) {
							next_x = 0;
						} else if (next_x >= WORLD_WIDTH) {
							next_x = WORLD_WIDTH-1;
						}
						int next_y = it->second + y_diff;
						if (next_y < 0) {
							next_y = 0;
						} else if (next_y >= WORLD_HEIGHT) {
							next_y = WORLD_HEIGHT-1;
						}

						if (!map_assigned[next_x][next_y]
								|| map_vals[next_x][next_y] == obs[1 + curr_index]) {
							next_possibilities.insert({next_x, next_y});
						}
					}
				}
				break;
			}
		}

		curr_index++;
		possibilities = next_possibilities;
	}

	if (curr_index < (int)actions.size()) {
		return false;
	} else {
		return true;
	}
}
