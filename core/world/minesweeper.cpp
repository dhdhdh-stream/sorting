#include "minesweeper.h"

#include <algorithm>
#include <iostream>

#include "globals.h"

using namespace std;

const int WIDTH = 0;
const int HEIGHT = 1;
const int NUM_MINES = 2;
const int STARTING_X = 3;
const int STARTING_Y = 4;
const vector<vector<int>> WORLD_SIZES = vector<vector<int>>{
	vector<int>{3, 3, 1, 1, 1},
	vector<int>{5, 5, 3, 2, 2},
	vector<int>{9, 9, 10, 4, 4},
	vector<int>{16, 16, 40, 8, 8},
	vector<int>{30, 16, 99, 15, 8}
};
const vector<double> WORLD_SIZE_PERCENTAGES = vector<double>{
	0.5,
	0.3,
	0.1,
	0.05,
	0.05,
};

Minesweeper::Minesweeper() {
	uniform_real_distribution<double> world_size_distribution(0.0, 1.0);
	double world_size_index = world_size_distribution(generator);
	for (int s_index = 0; s_index < (int)WORLD_SIZES.size(); s_index++) {
		world_size_index -= WORLD_SIZE_PERCENTAGES[s_index];
		if (world_size_index <= 0.0) {
			this->world_size = s_index;
			break;
		}
	}

	this->world = vector<vector<int>>(WORLD_SIZES[this->world_size][WIDTH], vector<int>(WORLD_SIZES[this->world_size][HEIGHT], 0));
	int num_mines = 0;
	uniform_int_distribution<int> x_distribution(0, WORLD_SIZES[this->world_size][WIDTH]-1);
	uniform_int_distribution<int> y_distribution(0, WORLD_SIZES[this->world_size][HEIGHT]-1);
	while (true) {
		int new_x = x_distribution(generator);
		int new_y = y_distribution(generator);

		if (this->world[new_x][new_y] != -1) {
			this->world[new_x][new_y] = -1;
			num_mines++;
			if (num_mines >= WORLD_SIZES[this->world_size][NUM_MINES]) {
				break;
			}
		}
	}

	for (int x_index = 0; x_index < WORLD_SIZES[this->world_size][WIDTH]; x_index++) {
		for (int y_index = 0; y_index < WORLD_SIZES[this->world_size][HEIGHT]; y_index++) {
			if (this->world[x_index][y_index] != -1) {
				int num_surrounding = 0;

				if (x_index > 0 && y_index < WORLD_SIZES[this->world_size][HEIGHT]-1) {
					if (this->world[x_index-1][y_index+1] == -1) {
						num_surrounding++;
					}
				}

				if (y_index < WORLD_SIZES[this->world_size][HEIGHT]-1) {
					if (this->world[x_index][y_index+1] == -1) {
						num_surrounding++;
					}
				}

				if (x_index < WORLD_SIZES[this->world_size][WIDTH]-1 && y_index < WORLD_SIZES[this->world_size][HEIGHT]-1) {
					if (this->world[x_index+1][y_index+1] == -1) {
						num_surrounding++;
					}
				}

				if (x_index < WORLD_SIZES[this->world_size][WIDTH]-1) {
					if (this->world[x_index+1][y_index] == -1) {
						num_surrounding++;
					}
				}

				if (x_index < WORLD_SIZES[this->world_size][WIDTH]-1 && y_index > 0) {
					if (this->world[x_index+1][y_index-1] == -1) {
						num_surrounding++;
					}
				}

				if (y_index > 0) {
					if (this->world[x_index][y_index-1] == -1) {
						num_surrounding++;
					}
				}

				if (x_index > 0 && y_index > 0) {
					if (this->world[x_index-1][y_index-1] == -1) {
						num_surrounding++;
					}
				}

				if (x_index > 0) {
					if (this->world[x_index-1][y_index] == -1) {
						num_surrounding++;
					}
				}

				this->world[x_index][y_index] = num_surrounding;
			}
		}
	}

	this->revealed = vector<vector<bool>>(WORLD_SIZES[this->world_size][WIDTH], vector<bool>(WORLD_SIZES[this->world_size][HEIGHT], false));
	this->flagged = vector<vector<bool>>(WORLD_SIZES[this->world_size][WIDTH], vector<bool>(WORLD_SIZES[this->world_size][HEIGHT], false));

	this->current_x = WORLD_SIZES[this->world_size][STARTING_X];
	this->current_y = WORLD_SIZES[this->world_size][STARTING_Y];

	this->ended = false;
}

int Minesweeper::num_actions() {
	return 6;
}

Action Minesweeper::random_action() {
	uniform_int_distribution<int> action_distribution(0, 5);
	return Action(action_distribution(generator));
}

double Minesweeper::get_observation() {
	if (this->current_x < 0
			|| this->current_x > WORLD_SIZES[this->world_size][WIDTH]-1
			|| this->current_y < 0
			|| this->current_y > WORLD_SIZES[this->world_size][HEIGHT]-1) {
		return -10.0;
	} else {
		if (this->current_x >= (int)this->flagged.size()
				|| this->current_y >= (int)this->flagged[this->current_x].size()) {
			cout << "this->world_size: " << this->world_size << endl;
			cout << "this->current_x: " << this->current_x << endl;
			cout << "this->flagged.size(): " << this->flagged.size() << endl;
			cout << "this->current_y: " << this->current_y << endl;
			cout << "this->flagged[this->current_x].size(): " << this->flagged[this->current_x].size() << endl;
		}
		if (this->flagged[this->current_x][this->current_y]) {
			return 10.0;
		} else if (this->revealed[this->current_x][this->current_y]) {
			return this->world[this->current_x][this->current_y];
		} else {
			return -5.0;
		}
	}
}

void Minesweeper::reveal_helper(int x, int y) {
	if (x < 0
			|| x > WORLD_SIZES[this->world_size][WIDTH]-1
			|| y < 0
			|| y > WORLD_SIZES[this->world_size][HEIGHT]-1) {
		return;
	} else if (this->revealed[x][y]) {
		return;
	} else {
		this->flagged[x][y] = false;
		this->revealed[x][y] = true;

		if (this->world[x][y] == -1) {
			this->ended = true;
		} else {
			if (this->world[x][y] == 0) {
				reveal_helper(x-1, y-1);
				reveal_helper(x-1, y);
				reveal_helper(x-1, y+1);
				reveal_helper(x, y+1);
				reveal_helper(x+1, y+1);
				reveal_helper(x+1, y);
				reveal_helper(x+1, y-1);
				reveal_helper(x, y-1);
			}
		}
	}
}

void Minesweeper::perform_action(Action action) {
	if (this->ended) {
		return;
	}

	if (action.move == MINESWEEPER_ACTION_UP) {
		if (this->current_y <= WORLD_SIZES[this->world_size][HEIGHT]-1) {
			this->current_y++;
		}
	} else if (action.move == MINESWEEPER_ACTION_RIGHT) {
		if (this->current_x <= WORLD_SIZES[this->world_size][WIDTH]-1) {
			this->current_x++;
		}
	} else if (action.move == MINESWEEPER_ACTION_DOWN) {
		if (this->current_y >= 0) {
			this->current_y--;
		}
	} else if (action.move == MINESWEEPER_ACTION_LEFT) {
		if (this->current_x >= 0) {
			this->current_x--;
		}
	} else if (action.move == MINESWEEPER_ACTION_CLICK) {
		reveal_helper(this->current_x, this->current_y);
	} else if (action.move == MINESWEEPER_ACTION_FLAG) {
		if (this->current_x >= 0
				&& this->current_x <= WORLD_SIZES[this->world_size][WIDTH]-1
				&& this->current_y >= 0
				&& this->current_y <= WORLD_SIZES[this->world_size][HEIGHT]-1) {
			if (!this->revealed[this->current_x][this->current_y]
					&& !this->flagged[this->current_x][this->current_y]) {
				this->flagged[this->current_x][this->current_y] = true;
			}
		}
	}
}

double Minesweeper::score_result(int num_process) {
	int num_correct = 0;
	for (int x_index = 0; x_index < WORLD_SIZES[this->world_size][WIDTH]; x_index++) {
		for (int y_index = 0; y_index < WORLD_SIZES[this->world_size][HEIGHT]; y_index++) {
			if (this->world[x_index][y_index] == -1) {
				if (this->flagged[x_index][y_index]) {
					num_correct++;
				}
			} else {
				if (this->revealed[x_index][y_index]) {
					num_correct++;
				}
			}
		}
	}

	return (num_correct - 0.01*num_process) / (WORLD_SIZES[this->world_size][WIDTH]*WORLD_SIZES[this->world_size][HEIGHT]);
}

Problem* Minesweeper::copy_and_reset() {
	Minesweeper* new_problem = new Minesweeper();

	new_problem->world_size = this->world_size;
	new_problem->world = this->world;

	new_problem->revealed = vector<vector<bool>>(WORLD_SIZES[this->world_size][WIDTH], vector<bool>(WORLD_SIZES[this->world_size][HEIGHT], false));
	new_problem->flagged = vector<vector<bool>>(WORLD_SIZES[this->world_size][WIDTH], vector<bool>(WORLD_SIZES[this->world_size][HEIGHT], false));

	new_problem->current_x = WORLD_SIZES[this->world_size][STARTING_X];
	new_problem->current_y = WORLD_SIZES[this->world_size][STARTING_Y];

	return new_problem;
}

void Minesweeper::print() {
	for (int y_index = WORLD_SIZES[this->world_size][HEIGHT]-1; y_index >= 0; y_index--) {
		for (int x_index = 0; x_index < WORLD_SIZES[this->world_size][WIDTH]; x_index++) {
			if (this->revealed[x_index][y_index]) {
				if (this->world[x_index][y_index] == -1) {
					cout << "X ";
				} else {
					cout << this->world[x_index][y_index] << " ";
				}
			} else if (this->flagged[x_index][y_index]) {
				if (this->world[x_index][y_index] == -1) {
					cout << "F ";
				} else {
					cout << "M ";
				}
			} else {
				cout << "- ";
			}
		}
		cout << endl;
	}

	cout << "current_x: " << current_x << endl;
	cout << "current_y: " << current_y << endl;
}
