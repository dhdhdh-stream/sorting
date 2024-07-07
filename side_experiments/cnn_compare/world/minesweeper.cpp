#include "minesweeper.h"

#include <iostream>

#include "globals.h"

using namespace std;

Minesweeper::Minesweeper() {
	while (true) {
		this->world = vector<vector<int>>(MINESWEEPER_WIDTH, vector<int>(MINESWEEPER_HEIGHT, 0));
		int num_mines = 0;
		uniform_int_distribution<int> x_distribution(0, MINESWEEPER_WIDTH-1);
		uniform_int_distribution<int> y_distribution(0, MINESWEEPER_HEIGHT-1);
		while (true) {
			int new_x = x_distribution(generator);
			int new_y = y_distribution(generator);

			if (this->world[new_x][new_y] != -1) {
				this->world[new_x][new_y] = -1;
				num_mines++;
				if (num_mines >= MINESWEEPER_NUM_MINES) {
					break;
				}
			}
		}

		for (int x_index = 0; x_index < MINESWEEPER_WIDTH; x_index++) {
			for (int y_index = 0; y_index < MINESWEEPER_HEIGHT; y_index++) {
				if (this->world[x_index][y_index] != -1) {
					int num_surrounding = 0;

					if (x_index > 0 && y_index < MINESWEEPER_HEIGHT-1) {
						if (this->world[x_index-1][y_index+1] == -1) {
							num_surrounding++;
						}
					}

					if (y_index < MINESWEEPER_HEIGHT-1) {
						if (this->world[x_index][y_index+1] == -1) {
							num_surrounding++;
						}
					}

					if (x_index < MINESWEEPER_WIDTH-1 && y_index < MINESWEEPER_HEIGHT-1) {
						if (this->world[x_index+1][y_index+1] == -1) {
							num_surrounding++;
						}
					}

					if (x_index < MINESWEEPER_WIDTH-1) {
						if (this->world[x_index+1][y_index] == -1) {
							num_surrounding++;
						}
					}

					if (x_index < MINESWEEPER_WIDTH-1 && y_index > 0) {
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

		if (this->world[MINESWEEPER_STARTING_X][MINESWEEPER_STARTING_Y] == 0) {
			break;
		}
	}

	this->revealed = vector<vector<bool>>(MINESWEEPER_WIDTH, vector<bool>(MINESWEEPER_HEIGHT, false));
	this->flagged = vector<vector<bool>>(MINESWEEPER_WIDTH, vector<bool>(MINESWEEPER_HEIGHT, false));

	this->current_x = MINESWEEPER_STARTING_X;
	this->current_y = MINESWEEPER_STARTING_Y;

	reveal_helper(MINESWEEPER_STARTING_X, MINESWEEPER_STARTING_Y);
}

double Minesweeper::get_observation_helper(int x, int y) {
	if (x < 0
			|| x > MINESWEEPER_WIDTH-1
			|| y < 0
			|| y > MINESWEEPER_HEIGHT-1) {
		return -20.0;
	} else {
		if (this->revealed[x][y]) {
			return 1.0 + this->world[x][y];
		} else if (this->flagged[x][y]) {
			return 20.0;
		} else {
			return -10.0;
		}
	}
}

void Minesweeper::reveal_helper(int x, int y) {
	if (x < 0
			|| x > MINESWEEPER_WIDTH-1
			|| y < 0
			|| y > MINESWEEPER_HEIGHT-1) {
		return;
	} else if (this->revealed[x][y]) {
		return;
	} else {
		this->revealed[x][y] = true;

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

double Minesweeper::score_result() {
	int curr_revealed = 0;
	double score = 1.0;
	for (int x_index = 0; x_index < MINESWEEPER_WIDTH; x_index++) {
		for (int y_index = 0; y_index < MINESWEEPER_HEIGHT; y_index++) {
			if (this->revealed[x_index][y_index]) {
				if (this->world[x_index][y_index] != -1) {
					curr_revealed++;
				}
			} else if (this->flagged[x_index][y_index]) {
				if (this->world[x_index][y_index] != -1) {
					score -= 1.0;
				} else {
					score += 0.2;
				}
			}
		}
	}

	score += 0.01*curr_revealed;

	return score;
}

void Minesweeper::print() {
	for (int y_index = MINESWEEPER_HEIGHT-1; y_index >= 0; y_index--) {
		for (int x_index = 0; x_index < MINESWEEPER_WIDTH; x_index++) {
			if (this->revealed[x_index][y_index]) {
				if (this->world[x_index][y_index] == -1) {
					cout << "X";
				} else {
					cout << this->world[x_index][y_index];
				}
			} else if (this->flagged[x_index][y_index]) {
				if (this->world[x_index][y_index] == -1) {
					cout << "F";
				} else {
					cout << "M";
				}
			} else {
				cout << "-";
			}

			if (x_index == this->current_x
					&& y_index == this->current_y) {
				cout << "<";
			} else {
				cout << " ";
			}
		}
		cout << endl;
	}

	cout << "current_x: " << this->current_x << endl;
	cout << "current_y: " << this->current_y << endl;
}
