#include "minesweeper_flag.h"

#include "globals.h"

using namespace std;

class Minesweeper {
public:
	vector<vector<int>> world;
	vector<vector<bool>> revealed;
	int current_x;
	int current_y;

	Minesweeper();

	vector<double> get_observations();

	double get_observation_helper(int x, int y);
	void reveal_helper(int x, int y);
};

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

	this->current_x = STARTING_X;
	this->current_y = STARTING_Y;

	reveal_helper(STARTING_X, STARTING_Y);
}

double Minesweeper::get_observation_helper(int x, int y) {
	if (x < 0
			|| x > WIDTH-1
			|| y < 0
			|| y > HEIGHT-1) {
		return -20.0;
	} else {
		if (this->revealed[x][y]) {
			return 1.0 + this->world[x][y];
		} else {
			return -10.0;
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

void Minesweeper::reveal_helper(int x, int y) {
	if (x < 0
			|| x > WIDTH-1
			|| y < 0
			|| y > HEIGHT-1) {
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

void MinesweeperFlag::get_train_instance(vector<double>& obs,
										 double& target_val) {
	Minesweeper problem;

	uniform_int_distribution<int> border_distribution(0, 3);
	if (border_distribution(generator) == 0) {
		vector<pair<int,int>> possibilities;
		for (int x_index = 0; x_index < WIDTH; x_index++) {
			for (int y_index = 0; y_index < HEIGHT; y_index++) {
				if (!problem.revealed[x_index][y_index]) {
					if (x_index < WIDTH-1
							&& problem.revealed[x_index+1][y_index]) {
						possibilities.push_back({x_index, y_index});
					} else if (x_index > 0
							&& problem.revealed[x_index-1][y_index]) {
						possibilities.push_back({x_index, y_index});
					} else if (y_index < HEIGHT-1
							&& problem.revealed[x_index][y_index+1]) {
						possibilities.push_back({x_index, y_index});
					} else if (y_index > 0
							&& problem.revealed[x_index][y_index-1]) {
						possibilities.push_back({x_index, y_index});
					}
				}
			}
		}

		uniform_int_distribution<int> distribution(0, possibilities.size()-1);
		int index = distribution(generator);
		problem.current_x = possibilities[index].first;
		problem.current_y = possibilities[index].second;
	} else {
		uniform_int_distribution<int> x_distribution(0, WIDTH-1);
		problem.current_x = x_distribution(generator);
		uniform_int_distribution<int> y_distribution(0, HEIGHT-1);
		problem.current_y = y_distribution(generator);
	}

	obs = problem.get_observations();
	if (problem.revealed[problem.current_x][problem.current_y]) {
		target_val = 0.0;
	} else {
		if (problem.world[problem.current_x][problem.current_y] == -1) {
			target_val = 0.2;
		} else {
			target_val = -1.0;
		}
	}
}

void MinesweeperFlag::get_test_instance(vector<double>& obs,
										double& target_val) {
	get_train_instance(obs,
					   target_val);
}
