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

const int FIELD_OF_VIEW = 2;

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

void Minesweeper::get_observations(vector<double>& obs,
								   vector<vector<double>>& locations) {
	for (int x_index = -FIELD_OF_VIEW; x_index <= FIELD_OF_VIEW; x_index++) {
		for (int y_index = -FIELD_OF_VIEW; y_index <= FIELD_OF_VIEW; y_index++) {
			obs.push_back(get_observation_helper(
				this->current_x + x_index,
				this->current_y + y_index));
			locations.push_back(vector<double>{
				this->current_x + x_index,
				this->current_y + y_index});
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

	switch (action.move) {
	case MINESWEEPER_ACTION_UP:
		this->current_y += action.distance;
		if (this->current_y > HEIGHT-1) {
			this->current_y = HEIGHT-1;
		}
		break;
	case MINESWEEPER_ACTION_RIGHT:
		this->current_x += action.distance;
		if (this->current_x > WIDTH-1) {
			this->current_x = WIDTH-1;
		}
		break;
	case MINESWEEPER_ACTION_DOWN:
		this->current_y -= action.distance;
		if (this->current_y < 0) {
			this->current_y = 0;
		}
		break;
	case MINESWEEPER_ACTION_LEFT:
		this->current_x -= action.distance;
		if (this->current_x < 0) {
			this->current_x = 0;
		}
		break;
	case MINESWEEPER_ACTION_CLICK:
		reveal_helper(this->current_x, this->current_y);
		break;
	case MINESWEEPER_ACTION_FLAG:
		if (this->current_x >= 0
				&& this->current_x < WIDTH
				&& this->current_y >= 0
				&& this->current_y < HEIGHT) {
			if (!this->revealed[this->current_x][this->current_y]
					&& !this->flagged[this->current_x][this->current_y]) {
				this->flagged[this->current_x][this->current_y] = true;
			}
		}
		break;
	case MINESWEEPER_ACTION_DOUBLECLICK:
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
		break;
	}
}

double Minesweeper::score_result(int num_analyze,
								 int num_actions) {
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

	score -= 0.00001*num_analyze;
	score -= 0.000005*num_actions;

	if (this->hit_mine) {
		score -= 1.0;
	}
	if (score < 0.0) {
		score = 0.0;
	}

	return score;
}

vector<double> Minesweeper::get_location() {
	return vector<double>{this->current_x, this->current_y};
}

void Minesweeper::return_to_location(vector<double>& location) {
	this->current_x = location[0];
	this->current_y = location[1];
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
	return 1;
}

int TypeMinesweeper::num_possible_actions() {
	return 7;
}

Action TypeMinesweeper::random_action() {
	uniform_int_distribution<int> action_distribution(0, 6);
	geometric_distribution<int> distance_distribution(0.4);
	return Action(action_distribution(generator), 1 + distance_distribution(generator));
}

vector<double> TypeMinesweeper::relative_to_world(
		vector<double>& comparison,
		vector<double>& relative_location) {
	return vector<double>{
		comparison[0] + relative_location[0],
		comparison[1] + relative_location[1]};
}

vector<double> TypeMinesweeper::world_to_relative(
		vector<double>& comparison,
		vector<double>& world_location) {
	return vector<double>{
		world_location[0] - comparison[0],
		world_location[1] - comparison[1]};
}
