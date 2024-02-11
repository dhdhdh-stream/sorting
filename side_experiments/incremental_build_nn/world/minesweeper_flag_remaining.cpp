#include "minesweeper_flag_remaining.h"

#include "constants.h"
#include "globals.h"
#include "minesweeper.h"

using namespace std;

MinesweeperFlagRemaining::MinesweeperFlagRemaining() {
	Minesweeper* minesweeper = new Minesweeper();

	minesweeper->world_size = 1;

	minesweeper->world = vector<vector<int>>(5, vector<int>(5, 0));
	int num_mines = 0;
	uniform_int_distribution<int> x_distribution(0, 4);
	uniform_int_distribution<int> y_distribution(0, 4);
	while (true) {
		int new_x = x_distribution(generator);
		int new_y = y_distribution(generator);

		if ((new_x != 2 || new_y != 2)
				&& minesweeper->world[new_x][new_y] != -1) {
			minesweeper->world[new_x][new_y] = -1;
			num_mines++;
			if (num_mines >= 4) {
				break;
			}
		}
	}

	for (int x_index = 0; x_index < 5; x_index++) {
		for (int y_index = 0; y_index < 5; y_index++) {
			if (minesweeper->world[x_index][y_index] != -1) {
				int num_surrounding = 0;

				if (x_index > 0 && y_index < 4) {
					if (minesweeper->world[x_index-1][y_index+1] == -1) {
						num_surrounding++;
					}
				}

				if (y_index < 4) {
					if (minesweeper->world[x_index][y_index+1] == -1) {
						num_surrounding++;
					}
				}

				if (x_index < 4 && y_index < 4) {
					if (minesweeper->world[x_index+1][y_index+1] == -1) {
						num_surrounding++;
					}
				}

				if (x_index < 4) {
					if (minesweeper->world[x_index+1][y_index] == -1) {
						num_surrounding++;
					}
				}

				if (x_index < 4 && y_index > 0) {
					if (minesweeper->world[x_index+1][y_index-1] == -1) {
						num_surrounding++;
					}
				}

				if (y_index > 0) {
					if (minesweeper->world[x_index][y_index-1] == -1) {
						num_surrounding++;
					}
				}

				if (x_index > 0 && y_index > 0) {
					if (minesweeper->world[x_index-1][y_index-1] == -1) {
						num_surrounding++;
					}
				}

				if (x_index > 0) {
					if (minesweeper->world[x_index-1][y_index] == -1) {
						num_surrounding++;
					}
				}

				minesweeper->world[x_index][y_index] = num_surrounding;
			}
		}
	}

	minesweeper->revealed = vector<vector<bool>>(5, vector<bool>(5, false));
	minesweeper->flagged = vector<vector<bool>>(5, vector<bool>(5, false));

	minesweeper->revealed[2][2] = true;

	uniform_int_distribution<int> distribution(0, 1);
	if (minesweeper->world[2][2] > 0 && distribution(generator) == 0) {
		uniform_int_distribution<int> flag_distribution(0, 3);
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				if (minesweeper->world[x_index][y_index] != -1) {
					minesweeper->revealed[x_index][y_index] = true;
				} else {
					if (flag_distribution(generator) == 0) {
						minesweeper->flagged[x_index][y_index] = true;
					}
				}
			}
		}
	} else {
		uniform_int_distribution<int> revealed_distribution(0, 2);
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				if (revealed_distribution(generator) != 0) {
					if (minesweeper->world[x_index][y_index] == -1) {
						minesweeper->flagged[x_index][y_index] = true;
					} else {
						minesweeper->revealed[x_index][y_index] = true;
					}
				}
			}
		}
	}

	minesweeper->current_x = 2;
	minesweeper->current_y = 2;

	this->problem = minesweeper;
}

MinesweeperFlagRemaining::~MinesweeperFlagRemaining() {
	delete this->problem;
}

void MinesweeperFlagRemaining::get_attention(
		vector<int>& types,
		vector<Action>& actions,
		vector<string>& scopes) {
	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_UP));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_UP));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_UP));
	scopes.push_back("");
}

void MinesweeperFlagRemaining::get_sequence(
		vector<int>& types,
		vector<Action>& actions,
		vector<string>& scopes) {
	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_UP));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_UP));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
	scopes.push_back("");

	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_UP));
	scopes.push_back("");
}

bool MinesweeperFlagRemaining::should_perform_sequence() {
	Minesweeper* minesweeper = (Minesweeper*)this->problem;

	int num_open = 0;
	int num_remaining = 0;

	if (!minesweeper->revealed[1][1]
			&& !minesweeper->flagged[1][1]) {
		num_open++;
	}
	if (minesweeper->world[1][1] == -1
			&& !minesweeper->flagged[1][1]) {
		num_remaining++;
	}

	if (!minesweeper->revealed[1][2]
			&& !minesweeper->flagged[1][2]) {
		num_open++;
	}
	if (minesweeper->world[1][2] == -1
			&& !minesweeper->flagged[1][2]) {
		num_remaining++;
	}

	if (!minesweeper->revealed[1][3]
			&& !minesweeper->flagged[1][3]) {
		num_open++;
	}
	if (minesweeper->world[1][3] == -1
			&& !minesweeper->flagged[1][3]) {
		num_remaining++;
	}

	if (!minesweeper->revealed[2][3]
			&& !minesweeper->flagged[2][3]) {
		num_open++;
	}
	if (minesweeper->world[2][3] == -1
			&& !minesweeper->flagged[2][3]) {
		num_remaining++;
	}

	if (!minesweeper->revealed[3][3]
			&& !minesweeper->flagged[3][3]) {
		num_open++;
	}
	if (minesweeper->world[3][3] == -1
			&& !minesweeper->flagged[3][3]) {
		num_remaining++;
	}

	if (!minesweeper->revealed[3][2]
			&& !minesweeper->flagged[3][2]) {
		num_open++;
	}
	if (minesweeper->world[3][2] == -1
			&& !minesweeper->flagged[3][2]) {
		num_remaining++;
	}

	if (!minesweeper->revealed[3][1]
			&& !minesweeper->flagged[3][1]) {
		num_open++;
	}
	if (minesweeper->world[3][1] == -1
			&& !minesweeper->flagged[3][1]) {
		num_remaining++;
	}

	if (!minesweeper->revealed[2][1]
			&& !minesweeper->flagged[2][1]) {
		num_open++;
	}
	if (minesweeper->world[2][1] == -1
			&& !minesweeper->flagged[2][1]) {
		num_remaining++;
	}

	if (num_open == num_remaining && num_remaining != 0) {
		return true;
	} else {
		return false;
	}
}

string MinesweeperFlagRemaining::get_name() {
	return "minesweeper_flag_remaining";
}
