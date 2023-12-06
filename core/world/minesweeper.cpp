#include "minesweeper.h"

#include <algorithm>
#include <iostream>

#include "globals.h"

using namespace std;

Minesweeper::Minesweeper() {
	this->world = vector<vector<int>>(9, vector<int>(9, 0));
	int num_mines = 0;
	uniform_int_distribution<int> x_distribution(0, 8);
	uniform_int_distribution<int> y_distribution(0, 8);
	while (true) {
		int new_x = x_distribution(generator);
		int new_y = y_distribution(generator);

		if (this->world[new_x][new_y] != -1) {
			this->world[new_x][new_y] = -1;
			num_mines++;
			if (num_mines > 10) {
				break;
			}
		}
	}

	for (int x_index = 0; x_index < 9; x_index++) {
		for (int y_index = 0; y_index < 9; y_index++) {
			if (this->world[x_index][y_index] != -1) {
				int num_surrounding = 0;

				if (x_index > 0 && y_index < 8) {
					if (this->world[x_index-1][y_index+1] == -1) {
						num_surrounding++;
					}
				}

				if (y_index < 8) {
					if (this->world[x_index][y_index+1] == -1) {
						num_surrounding++;
					}
				}

				if (x_index < 8 && y_index < 8) {
					if (this->world[x_index+1][y_index+1] == -1) {
						num_surrounding++;
					}
				}

				if (x_index < 8) {
					if (this->world[x_index+1][y_index] == -1) {
						num_surrounding++;
					}
				}

				if (x_index < 8 && y_index > 0) {
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
			}
		}
	}

	this->revealed = vector<vector<bool>>(9, vector<bool>(9, false));

	this->current_x = 4;
	this->current_y = 4;

	this->num_revealed = 0;

	this->ended = false;
}

Action Minesweeper::random_action() {
	uniform_int_distribution<int> action_distribution(0, 4);
	return Action(action_distribution(generator));
}

double Minesweeper::get_observation() {
	if (this->current_x < 0
			|| this->current_x > 8
			|| this->current_y < 0
			|| this->current_y > 8) {
		return -10.0;
	} else {
		if (this->revealed[this->current_x][this->current_y]) {
			return this->world[this->current_x][this->current_y];
		} else {
			return -5.0;
		}
	}
}

void Minesweeper::perform_action(Action action) {
	if (this->ended) {
		return;
	}

	if (action.move == MINESWEEPER_ACTION_UP) {
		if (this->current_y <= 8) {
			this->current_y++;
		}
	} else if (action.move == MINESWEEPER_ACTION_RIGHT) {
		if (this->current_x <= 8) {
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
		if (this->current_x >= 0
				|| this->current_x <= 8
				|| this->current_y >= 0
				|| this->current_y <= 8) {
			if (!this->revealed[this->current_x][this->current_y]) {
				if (this->world[this->current_x][this->current_y] == -1) {
					this->ended = true;
				} else {
					this->revealed[this->current_x][this->current_y] = true;
					this->num_revealed++;
					if (this->num_revealed >= 71) {
						this->ended = true;
					}
				}
			}
		}
	}
}

double Minesweeper::score_result() {
	if (this->num_revealed >= 71) {
		return 10.0;
	} else {
		return this->num_revealed/10.0;
	}
}

Problem* Minesweeper::copy_and_reset() {
	Minesweeper* new_problem = new Minesweeper();

	new_problem->world = this->world;

	return new_problem;
}

void Minesweeper::print() {
	for (int y_index = 8; y_index >= 0; y_index--) {
		for (int x_index = 0; x_index < 9; x_index++) {
			if (this->revealed[x_index][y_index]) {
				cout << this->world[x_index][y_index] << " ";
			} else {
				cout << "- ";
			}
			cout << endl;
		}
	}

	cout << "current_x: " << current_x << endl;
	cout << "current_y: " << current_y << endl;
}
