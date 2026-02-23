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

	this->hit_mine = false;

	reveal_helper(STARTING_X, STARTING_Y);

	this->starting_score = score_result_helper();
}

double Minesweeper::get_observation_helper(int x, int y) {
	if (x < 0
			|| x > WIDTH-1
			|| y < 0
			|| y > HEIGHT-1) {
		return -20.0;
	} else {
		if (this->hit_mine) {
			if (this->revealed[x][y]) {
				if (this->world[x][y] == -1) {
					return -3.0;
				} else {
					return 1.0 + this->world[x][y];
				}
			} else if (this->flagged[x][y]) {
				if (this->world[x][y] != -1) {
					return -5.0;
				} else {
					return 20.0;
				}
			} else {
				return -10.0;
			}
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
}

vector<double> Minesweeper::get_observations() {
	vector<double> obs;

	// obs.push_back(get_observation_helper(this->current_x, this->current_y));

	for (int x_index = -2; x_index <= 2; x_index++) {
		for (int y_index = -2; y_index <= 2; y_index++) {
			obs.push_back(get_observation_helper(
				this->current_x + x_index,
				this->current_y + y_index
			));
		}
	}

	return obs;
}

vector<double> Minesweeper::get_signal_obs() {
	vector<double> obs;

	obs.push_back(get_observation_helper(this->current_x, this->current_y));

	// for (int x_index = -2; x_index <= 2; x_index++) {
	// 	for (int y_index = -2; y_index <= 2; y_index++) {
	// 		obs.push_back(get_observation_helper(
	// 			this->current_x + x_index,
	// 			this->current_y + y_index
	// 		));
	// 	}
	// }

	return obs;
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

void Minesweeper::perform_action(int action) {
	switch (action) {
	case MINESWEEPER_ACTION_UP:
		this->current_y++;
		// if (this->current_y > HEIGHT) {
		// 	this->current_y = HEIGHT;
		// }
		if (this->current_y > HEIGHT-1) {
			this->current_y = HEIGHT-1;
		}
		break;
	case MINESWEEPER_ACTION_RIGHT:
		this->current_x++;
		// if (this->current_x > WIDTH) {
		// 	this->current_x = WIDTH;
		// }
		if (this->current_x > WIDTH-1) {
			this->current_x = WIDTH-1;
		}
		break;
	case MINESWEEPER_ACTION_DOWN:
		this->current_y--;
		// if (this->current_y < -1) {
		// 	this->current_y = -1;
		// }
		if (this->current_y < 0) {
			this->current_y = 0;
		}
		break;
	case MINESWEEPER_ACTION_LEFT:
		this->current_x--;
		// if (this->current_x < -1) {
		// 	this->current_x = -1;
		// }
		if (this->current_x < 0) {
			this->current_x = 0;
		}
		break;
	case MINESWEEPER_ACTION_CLICK:
		if (!this->hit_mine) {
			reveal_helper(this->current_x, this->current_y);
		}
		break;
	case MINESWEEPER_ACTION_FLAG:
		if (!this->hit_mine) {
			if (this->current_x >= 0
					&& this->current_x < WIDTH
					&& this->current_y >= 0
					&& this->current_y < HEIGHT) {
				if (!this->revealed[this->current_x][this->current_y]
						&& !this->flagged[this->current_x][this->current_y]) {
					this->flagged[this->current_x][this->current_y] = true;
				}
			}
		}
		break;
	case MINESWEEPER_ACTION_DOUBLECLICK:
		if (!this->hit_mine) {
			if (this->current_x >= 0
					&& this->current_x < WIDTH
					&& this->current_y >= 0
					&& this->current_y < HEIGHT) {
				if (this->revealed[this->current_x][this->current_y]
						&& this->world[this->current_x][this->current_y] > 0) {
					int num_surrounding = 0;

					if (this->current_x > 0 && this->current_y < HEIGHT-1) {
						if (this->flagged[this->current_x-1][this->current_y+1]) {
							num_surrounding++;
						}
					}

					if (this->current_y < HEIGHT-1) {
						if (this->flagged[this->current_x][this->current_y+1]) {
							num_surrounding++;
						}
					}

					if (this->current_x < WIDTH-1 && this->current_y < HEIGHT-1) {
						if (this->flagged[this->current_x+1][this->current_y+1]) {
							num_surrounding++;
						}
					}

					if (this->current_x < WIDTH-1) {
						if (this->flagged[this->current_x+1][this->current_y]) {
							num_surrounding++;
						}
					}

					if (this->current_x < WIDTH-1 && this->current_y > 0) {
						if (this->flagged[this->current_x+1][this->current_y-1]) {
							num_surrounding++;
						}
					}

					if (this->current_y > 0) {
						if (this->flagged[this->current_x][this->current_y-1]) {
							num_surrounding++;
						}
					}

					if (this->current_x > 0 && this->current_y > 0) {
						if (this->flagged[this->current_x-1][this->current_y-1]) {
							num_surrounding++;
						}
					}

					if (this->current_x > 0) {
						if (this->flagged[this->current_x-1][this->current_y]) {
							num_surrounding++;
						}
					}

					if (num_surrounding == this->world[this->current_x][this->current_y]) {
						if (this->current_x > 0 && this->current_y > 0) {
							if (!this->revealed[this->current_x-1][this->current_y-1]
									&& !this->flagged[this->current_x-1][this->current_y-1]) {
								reveal_helper(this->current_x-1, this->current_y-1);
							}
						}
						if (this->current_x > 0) {
							if (!this->revealed[this->current_x-1][this->current_y]
									&& !this->flagged[this->current_x-1][this->current_y]) {
								reveal_helper(this->current_x-1, this->current_y);
							}
						}
						if (this->current_x > 0 && this->current_y < HEIGHT-1) {
							if (!this->revealed[this->current_x-1][this->current_y+1]
									&& !this->flagged[this->current_x-1][this->current_y+1]) {
								reveal_helper(this->current_x-1, this->current_y+1);
							}
						}
						if (this->current_y < HEIGHT-1) {
							if (!this->revealed[this->current_x][this->current_y+1]
									&& !this->flagged[this->current_x][this->current_y+1]) {
								reveal_helper(this->current_x, this->current_y+1);
							}
						}
						if (this->current_x < WIDTH-1 && this->current_y < HEIGHT-1) {
							if (!this->revealed[this->current_x+1][this->current_y+1]
									&& !this->flagged[this->current_x+1][this->current_y+1]) {
								reveal_helper(this->current_x+1, this->current_y+1);
							}
						}
						if (this->current_x < WIDTH-1) {
							if (!this->revealed[this->current_x+1][this->current_y]
									&& !this->flagged[this->current_x+1][this->current_y]) {
								reveal_helper(this->current_x+1, this->current_y);
							}
						}
						if (this->current_x < WIDTH-1 && this->current_y > 0) {
							if (!this->revealed[this->current_x+1][this->current_y-1]
									&& !this->flagged[this->current_x+1][this->current_y-1]) {
								reveal_helper(this->current_x+1, this->current_y-1);
							}
						}
						if (this->current_y > 0) {
							if (!this->revealed[this->current_x][this->current_y-1]
									&& !this->flagged[this->current_x][this->current_y-1]) {
								reveal_helper(this->current_x, this->current_y-1);
							}
						}
					}
				}
			}
		}
		break;
	}
}

double Minesweeper::score_result_helper() {
	int curr_revealed = 0;
	int num_mines = 0;
	double score = 1.0;
	for (int x_index = 0; x_index < WIDTH; x_index++) {
		for (int y_index = 0; y_index < HEIGHT; y_index++) {
			if (this->revealed[x_index][y_index]) {
				if (this->world[x_index][y_index] != -1) {
					curr_revealed++;
				}
			} else if (this->flagged[x_index][y_index]) {
				if (this->world[x_index][y_index] != -1) {
					score -= 1.0;
				} else {
					num_mines++;
					score += 0.2;
				}
			}
		}
	}

	if (curr_revealed == 71
			&& num_mines == 10
			&& !this->hit_mine) {
		score += 10.0;
	}

	score += 0.01*curr_revealed;

	if (this->hit_mine) {
		score -= 1.0;
	}

	return score;
}

double Minesweeper::score_result() {
	// double final_score = score_result_helper();
	// return final_score - this->starting_score;

	return score_result_helper();
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

Problem* Minesweeper::copy_snapshot() {
	Minesweeper* new_problem = new Minesweeper();

	new_problem->world = this->world;
	new_problem->revealed = this->revealed;
	new_problem->flagged = this->flagged;
	new_problem->current_x = this->current_x;
	new_problem->current_y = this->current_y;
	new_problem->hit_mine = this->hit_mine;

	return new_problem;
}

void Minesweeper::print() {
	for (int y_index = HEIGHT-1; y_index >= 0; y_index--) {
		for (int x_index = 0; x_index < WIDTH; x_index++) {
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

Problem* TypeMinesweeper::get_problem() {
	return new Minesweeper();
}

int TypeMinesweeper::num_obs() {
	// return 1;
	return 25;
}

int TypeMinesweeper::num_possible_actions() {
	return 7;
}
