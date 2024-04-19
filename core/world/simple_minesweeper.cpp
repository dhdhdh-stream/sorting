#include "simple_minesweeper.h"

#include <algorithm>
#include <iostream>

#include "globals.h"

using namespace std;

const int WIDTH = 9;
const int HEIGHT = 9;
const int NUM_MINES = 10;
const int STARTING_X = 4;
const int STARTING_Y = 4;

SimpleMinesweeper::SimpleMinesweeper() {
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

	this->starting_revealed = 0;
	for (int x_index = 0; x_index < WIDTH; x_index++) {
		for (int y_index = 0; y_index < HEIGHT; y_index++) {
			if (this->revealed[x_index][y_index]) {
				this->starting_revealed++;
			}
		}
	}

	this->num_actions = 0;
}

int SimpleMinesweeper::num_obs() {
	return 9;
}

int SimpleMinesweeper::num_possible_actions() {
	return 5;
}

Action SimpleMinesweeper::random_action() {
	uniform_int_distribution<int> action_distribution(0, 4);
	return Action(action_distribution(generator));
}

double SimpleMinesweeper::get_observation_helper(int x, int y) {
	if (x < 0
			|| x > WIDTH-1
			|| y < 0
			|| y > HEIGHT-1) {
		return -10.0;
	} else {
		if (this->revealed[x][y]) {
			return this->world[x][y];
		} else if (this->flagged[x][y]) {
			return 10.0;
		} else {
			return -5.0;
		}
	}
}

vector<double> SimpleMinesweeper::get_observations() {
	vector<double> obs;

	obs.push_back(get_observation_helper(this->current_x, this->current_y));

	obs.push_back(get_observation_helper(this->current_x-1, this->current_y+1));
	obs.push_back(get_observation_helper(this->current_x, this->current_y+1));
	obs.push_back(get_observation_helper(this->current_x+1, this->current_y+1));
	obs.push_back(get_observation_helper(this->current_x+1, this->current_y));
	obs.push_back(get_observation_helper(this->current_x+1, this->current_y-1));
	obs.push_back(get_observation_helper(this->current_x, this->current_y-1));
	obs.push_back(get_observation_helper(this->current_x-1, this->current_y-1));
	obs.push_back(get_observation_helper(this->current_x-1, this->current_y));

	return obs;
}

void SimpleMinesweeper::reveal_helper(int x, int y) {
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

void SimpleMinesweeper::perform_action(Action action) {
	if (action.move != ACTION_NOOP) {
		this->num_actions++;
	}

	if (this->hit_mine) {
		return;
	}

	switch (action.move) {
	case SIMPLE_MINESWEEPER_ACTION_UP:
		if (this->current_y < HEIGHT-1) {
			this->current_y++;
		}
		break;
	case SIMPLE_MINESWEEPER_ACTION_RIGHT:
		if (this->current_x < WIDTH-1) {
			this->current_x++;
		}
		break;
	case SIMPLE_MINESWEEPER_ACTION_DOWN:
		if (this->current_y > 0) {
			this->current_y--;
		}
		break;
	case SIMPLE_MINESWEEPER_ACTION_LEFT:
		if (this->current_x > 0) {
			this->current_x--;
		}
		break;
	case SIMPLE_MINESWEEPER_ACTION_EVAL:
		if (this->revealed[this->current_x][this->current_y]
				&& this->world[this->current_x][this->current_y] != 0) {
			int num_mines = this->world[this->current_x][this->current_y];

			int num_flagged = 0;
			int num_revealed = 0;

			// top left
			if (this->current_x > 0 && this->current_y < HEIGHT-1) {
				if (this->flagged[this->current_x-1][this->current_y+1]) {
					num_flagged++;
				}
				if (this->revealed[this->current_x-1][this->current_y+1]) {
					num_revealed++;
				}
			}

			// top
			if (this->current_y < HEIGHT-1) {
				if (this->flagged[this->current_x][this->current_y+1]) {
					num_flagged++;
				}
				if (this->revealed[this->current_x][this->current_y+1]) {
					num_revealed++;
				}
			}

			// top right
			if (this->current_x < WIDTH-1 && this->current_y < HEIGHT-1) {
				if (this->flagged[this->current_x+1][this->current_y+1]) {
					num_flagged++;
				}
				if (this->revealed[this->current_x+1][this->current_y+1]) {
					num_revealed++;
				}
			}

			// right
			if (this->current_x < WIDTH-1) {
				if (this->flagged[this->current_x+1][this->current_y]) {
					num_flagged++;
				}
				if (this->revealed[this->current_x+1][this->current_y]) {
					num_revealed++;
				}
			}

			// bottom right
			if (this->current_x < WIDTH-1 && this->current_y > 0) {
				if (this->flagged[this->current_x+1][this->current_y-1]) {
					num_flagged++;
				}
				if (this->revealed[this->current_x+1][this->current_y-1]) {
					num_revealed++;
				}
			}

			// bottom
			if (this->current_y > 0) {
				if (this->flagged[this->current_x][this->current_y-1]) {
					num_flagged++;
				}
				if (this->revealed[this->current_x][this->current_y-1]) {
					num_revealed++;
				}
			}

			// bottom left
			if (this->current_x > 0 && this->current_y > 0) {
				if (this->flagged[this->current_x-1][this->current_y-1]) {
					num_flagged++;
				}
				if (this->revealed[this->current_x-1][this->current_y-1]) {
					num_revealed++;
				}
			}

			// left
			if (this->current_x > 0) {
				if (this->flagged[this->current_x-1][this->current_y]) {
					num_flagged++;
				}
				if (this->revealed[this->current_x-1][this->current_y]) {
					num_revealed++;
				}
			}

			if (num_flagged + num_revealed != 8) {
				if (num_mines == num_flagged) {
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
				} else if (num_revealed + num_mines == 8) {
					// top left
					if (this->current_x > 0 && this->current_y < HEIGHT-1) {
						if (!this->flagged[this->current_x-1][this->current_y+1]
								&& !this->revealed[this->current_x-1][this->current_y+1]) {
							this->flagged[this->current_x-1][this->current_y+1] = true;
						}
					}

					// top
					if (this->current_y < HEIGHT-1) {
						if (!this->flagged[this->current_x][this->current_y+1]
								&& !this->revealed[this->current_x][this->current_y+1]) {
							this->flagged[this->current_x][this->current_y+1] = true;
						}
					}

					// top right
					if (this->current_x < WIDTH-1 && this->current_y < HEIGHT-1) {
						if (!this->flagged[this->current_x+1][this->current_y+1]
								&& !this->revealed[this->current_x+1][this->current_y+1]) {
							this->flagged[this->current_x+1][this->current_y+1] = true;
						}
					}

					// right
					if (this->current_x < WIDTH-1) {
						if (!this->flagged[this->current_x+1][this->current_y]
								&& !this->revealed[this->current_x+1][this->current_y]) {
							this->flagged[this->current_x+1][this->current_y] = true;
						}
					}

					// bottom right
					if (this->current_x < WIDTH-1 && this->current_y > 0) {
						if (!this->flagged[this->current_x+1][this->current_y-1]
								&& !this->revealed[this->current_x+1][this->current_y-1]) {
							this->flagged[this->current_x+1][this->current_y-1] = true;
						}
					}

					// bottom
					if (this->current_y > 0) {
						if (!this->flagged[this->current_x][this->current_y-1]
								&& !this->revealed[this->current_x][this->current_y-1]) {
							this->flagged[this->current_x][this->current_y-1] = true;
						}
					}

					// bottom left
					if (this->current_x > 0 && this->current_y > 0) {
						if (!this->flagged[this->current_x-1][this->current_y-1]
								&& !this->revealed[this->current_x-1][this->current_y-1]) {
							this->flagged[this->current_x-1][this->current_y-1] = true;
						}
					}

					// left
					if (this->current_x > 0) {
						if (!this->flagged[this->current_x-1][this->current_y]
								&& !this->revealed[this->current_x-1][this->current_y]) {
							this->flagged[this->current_x-1][this->current_y] = true;
						}
					}
				}
			}
		}
		break;
	}
}

double SimpleMinesweeper::score_result() {
	int curr_revealed = 0;
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
					score += 0.1;
				}
			}
		}
	}

	score += 0.01*(curr_revealed - this->starting_revealed);

	if (this->num_actions > 20) {
		score -= 0.0003*(this->num_actions-20);
	}

	if (this->hit_mine) {
		score -= 1.0;
	}
	if (score < 0.0) {
		score = 0.0;
	}

	return score;
}

Problem* SimpleMinesweeper::copy_and_reset() {
	SimpleMinesweeper* new_problem = new SimpleMinesweeper();

	new_problem->world = this->world;

	new_problem->revealed = vector<vector<bool>>(WIDTH, vector<bool>(HEIGHT, false));
	new_problem->flagged = vector<vector<bool>>(WIDTH, vector<bool>(HEIGHT, false));

	new_problem->current_x = STARTING_X;
	new_problem->current_y = STARTING_Y;

	new_problem->reveal_helper(STARTING_X, STARTING_Y);

	new_problem->starting_revealed = this->starting_revealed;

	return new_problem;
}

void SimpleMinesweeper::print() {
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

	cout << "num_actions: " << this->num_actions << endl;
}
