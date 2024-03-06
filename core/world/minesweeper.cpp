#include "minesweeper.h"

#include <algorithm>
#include <iostream>

#include "globals.h"

using namespace std;

const int WIDTH = 9;
const int HEIGHT = 9;
const int NUM_MINES = 10;
const int STARTING_X = 4;
const int STARTING_Y = 4;

Minesweeper::Minesweeper() {
	while (true) {
		this->world = vector<vector<int>>(WIDTH, vector<int>(HEIGHT, 0));
		int num_mines = 0;
		uniform_int_distribution<int> x_distribution(0, WIDTH-1);
		uniform_int_distribution<int> y_distribution(0, HEIGHT-1);
		while (true) {
			int new_x = x_distribution(generator);
			int new_y = y_distribution(generator);

			if (this->world[new_x][new_y] != -1) {
				this->world[new_x][new_y] = -1;
				num_mines++;
				if (num_mines >= NUM_MINES) {
					break;
				}
			}
		}

		for (int x_index = 0; x_index < WIDTH; x_index++) {
			for (int y_index = 0; y_index < HEIGHT; y_index++) {
				if (this->world[x_index][y_index] != -1) {
					int num_surrounding = 0;

					if (x_index > 0 && y_index < HEIGHT-1) {
						if (this->world[x_index-1][y_index+1] == -1) {
							num_surrounding++;
						}
					}

					if (y_index < HEIGHT-1) {
						if (this->world[x_index][y_index+1] == -1) {
							num_surrounding++;
						}
					}

					if (x_index < WIDTH-1 && y_index < HEIGHT-1) {
						if (this->world[x_index+1][y_index+1] == -1) {
							num_surrounding++;
						}
					}

					if (x_index < WIDTH-1) {
						if (this->world[x_index+1][y_index] == -1) {
							num_surrounding++;
						}
					}

					if (x_index < WIDTH-1 && y_index > 0) {
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

		if (this->world[STARTING_X][STARTING_Y] == 0) {
			break;
		}
	}

	this->revealed = vector<vector<bool>>(WIDTH, vector<bool>(HEIGHT, false));
	this->flagged = vector<vector<bool>>(WIDTH, vector<bool>(HEIGHT, false));

	this->current_x = STARTING_X;
	this->current_y = STARTING_Y;

	this->num_correct = 0;

	this->hit_mine = false;

	reveal_helper(STARTING_X, STARTING_Y);
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
			|| this->current_x > WIDTH-1
			|| this->current_y < 0
			|| this->current_y > HEIGHT-1) {
		return -10.0;
	} else {
		if (this->revealed[this->current_x][this->current_y]) {
			return this->world[this->current_x][this->current_y];
		} else if (this->flagged[this->current_x][this->current_y]) {
			return 10.0;
		} else {
			return -5.0;
		}
	}
}

void Minesweeper::reveal_helper(int x, int y) {
	if (x < 0
			|| x > WIDTH-1
			|| y < 0
			|| y > HEIGHT-1) {
		return;
	} else if (this->revealed[x][y]) {
		return;
	} else {
		this->flagged[x][y] = false;
		this->revealed[x][y] = true;

		if (this->world[x][y] == -1) {
			this->hit_mine = true;
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
	if (this->hit_mine) {
		return;
	}

	if (action.move == MINESWEEPER_ACTION_UP) {
		if (this->current_y <= HEIGHT-1) {
			this->current_y++;
		}
	} else if (action.move == MINESWEEPER_ACTION_RIGHT) {
		if (this->current_x <= WIDTH-1) {
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
				&& this->current_x <= WIDTH-1
				&& this->current_y >= 0
				&& this->current_y <= HEIGHT-1) {
			if (!this->revealed[this->current_x][this->current_y]
					&& !this->flagged[this->current_x][this->current_y]) {
				this->flagged[this->current_x][this->current_y] = true;
			}
		}
	}
}

double Minesweeper::score_result() {
	double score = 1.0;
	for (int x_index = 0; x_index < WIDTH; x_index++) {
		for (int y_index = 0; y_index < HEIGHT; y_index++) {
			if (this->revealed[x_index][y_index]) {
				if (this->world[x_index][y_index] != -1) {
					score += 0.01;
				}
			} else if (this->flagged[x_index][y_index]) {
				if (this->world[x_index][y_index] != -1) {
					score -= 1.0;
				} else {
					score += 0.1;
				}
			}
		}
	}

	if (this->hit_mine) {
		score -= 1.0;
	}
	if (score < 0.0) {
		score = 0.0;
	}

	return score;
}

Problem* Minesweeper::copy_and_reset() {
	Minesweeper* new_problem = new Minesweeper();

	new_problem->world = this->world;

	new_problem->revealed = vector<vector<bool>>(WIDTH, vector<bool>(HEIGHT, false));
	new_problem->flagged = vector<vector<bool>>(WIDTH, vector<bool>(HEIGHT, false));

	new_problem->current_x = STARTING_X;
	new_problem->current_y = STARTING_Y;

	new_problem->reveal_helper(STARTING_X, STARTING_Y);

	return new_problem;
}

void Minesweeper::print() {
	for (int y_index = HEIGHT-1; y_index >= 0; y_index--) {
		for (int x_index = 0; x_index < WIDTH; x_index++) {
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
