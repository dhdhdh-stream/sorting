// - difficult to detect errors early
//   - and when can't do so, then have to expand a lot

#include "assign_node.h"

#include <iostream>

#include "globals.h"
#include "mapping_helpers.h"
#include "world.h"

using namespace std;

/**
 * - assume immediately following init
 */
bool AssignNode::solve(vector<int>& obs,
					   vector<int>& actions,
					   int curr_index,
					   int curr_x,
					   int curr_y,
					   vector<vector<int>>& map_vals,
					   vector<vector<bool>>& map_assigned) {
	if (curr_x < 0) {
		curr_x = 0;
	} else if (curr_x >= WORLD_WIDTH) {
		curr_x = WORLD_WIDTH-1;
	}
	if (curr_y < 0) {
		curr_y = 0;
	} else if (curr_y >= WORLD_HEIGHT) {
		curr_y = WORLD_HEIGHT-1;
	}

	vector<vector<int>> next_map_vals = map_vals;
	vector<vector<bool>> next_map_assigned = map_assigned;
	if (next_map_assigned[curr_x][curr_y]) {
		if (next_map_vals[curr_x][curr_y] == obs[1 + curr_index]) {
			// no check needed
		} else {
			// // temp
			// cout << "assign fail " << curr_index << endl;

			this->is_fail = true;
			return false;
		}
	} else {
		next_map_vals[curr_x][curr_y] = obs[1 + curr_index];
		next_map_assigned[curr_x][curr_y] = true;

		bool is_valid = check_valid(obs,
									actions,
									next_map_vals,
									next_map_assigned);

		if (!is_valid) {
			// // temp
			// cout << "check fail " << curr_index << endl;

			this->is_fail = true;
			return false;
		}
	}

	if (curr_index >= (int)actions.size()) {
		for (int x_index = 0; x_index < WORLD_WIDTH; x_index++) {
			for (int y_index = 0; y_index < WORLD_HEIGHT; y_index++) {
				cout << next_map_vals[x_index][y_index] << " ";
			}
			cout << endl;
		}

		this->is_fail = false;
		return true;
	}

	switch (actions[curr_index]) {
	case ACTION_UP:
		for (int x_diff = -1; x_diff <= 1; x_diff++) {
			for (int y_diff = 1; y_diff <= 2; y_diff++) {
				this->move.push_back({x_diff, y_diff});
				this->children.push_back(NULL);
			}
		}
		break;
	case ACTION_RIGHT:
		for (int x_diff = 1; x_diff <= 2; x_diff++) {
			for (int y_diff = -1; y_diff <= 1; y_diff++) {
				this->move.push_back({x_diff, y_diff});
				this->children.push_back(NULL);
			}
		}
		break;
	case ACTION_DOWN:
		for (int x_diff = -1; x_diff <= 1; x_diff++) {
			for (int y_diff = -2; y_diff <= -1; y_diff++) {
				this->move.push_back({x_diff, y_diff});
				this->children.push_back(NULL);
			}
		}
		break;
	case ACTION_LEFT:
		for (int x_diff = -2; x_diff <= -1; x_diff++) {
			for (int y_diff = -1; y_diff <= 1; y_diff++) {
				this->move.push_back({x_diff, y_diff});
				this->children.push_back(NULL);
			}
		}
		break;
	}

	vector<int> remaining_indexes(this->children.size());
	for (int i_index = 0; i_index < (int)this->children.size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}

	bool children_is_success = false;
	while (remaining_indexes.size() > 0) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);

		int c_index = remaining_indexes[index];

		this->children[c_index] = new AssignNode();
		bool is_success = this->children[c_index]->solve(obs,
														 actions,
														 curr_index + 1,
														 curr_x + this->move[c_index].first,
														 curr_y + this->move[c_index].second,
														 next_map_vals,
														 next_map_assigned);
		if (is_success) {
			children_is_success = true;
			break;
		} else {
			delete this->children[c_index];
			this->children[c_index] = NULL;
		}

		remaining_indexes.erase(remaining_indexes.begin() + index);
	}

	this->is_fail = !children_is_success;
	return children_is_success;
}
