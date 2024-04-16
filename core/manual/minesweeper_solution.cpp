#include "minesweeper_solution.h"

#include "minesweeper.h"

using namespace std;

MinesweeperSolution::MinesweeperSolution() {
	this->state = MINESWEEPER_SOLUTION_STATE_TO_BOTTOM;

	this->iter_made_progress = false;
}

MinesweeperSolution::~MinesweeperSolution() {
	// do nothing
}

void MinesweeperSolution::step(vector<double>& observations,
							   bool& done,
							   vector<Action>& actions) {
	bool step_done = false;
	while (true) {
		switch (this->state) {
		case MINESWEEPER_SOLUTION_STATE_TO_BOTTOM:
			if (observations[6] == -10.0) {
				this->state = MINESWEEPER_SOLUTION_STATE_TO_RIGHT;
			} else {
				done = false;
				actions.push_back(Action(MINESWEEPER_ACTION_DOWN));

				step_done = true;
			}
			break;
		case MINESWEEPER_SOLUTION_STATE_TO_RIGHT:
			if (observations[4] == -10.0) {
				this->state = MINESWEEPER_SOLUTION_STATE_TRAVERSE_LEFT;
			} else {
				done = false;
				actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));

				step_done = true;
			}

			break;
		case MINESWEEPER_SOLUTION_STATE_TRAVERSE_LEFT:
			if (observations[0] > 0.0 && observations[0] < 10.0) {
				int num_mines = (int)observations[0];

				int num_flagged = 0;
				int num_revealed = 0;

				// top left
				if (observations[1] == 10.0) {
					num_flagged++;
				} else if (observations[1] != -5.0) {
					num_revealed++;
				}

				// top
				if (observations[2] == 10.0) {
					num_flagged++;
				} else if (observations[2] != -5.0) {
					num_revealed++;
				}

				// top right
				if (observations[3] == 10.0) {
					num_flagged++;
				} else if (observations[3] != -5.0) {
					num_revealed++;
				}

				// right
				if (observations[4] == 10.0) {
					num_flagged++;
				} else if (observations[4] != -5.0) {
					num_revealed++;
				}

				// bottom right
				if (observations[5] == 10.0) {
					num_flagged++;
				} else if (observations[5] != -5.0) {
					num_revealed++;
				}

				// bottom
				if (observations[6] == 10.0) {
					num_flagged++;
				} else if (observations[6] != -5.0) {
					num_revealed++;
				}

				// bottom left
				if (observations[7] == 10.0) {
					num_flagged++;
				} else if (observations[7] != -5.0) {
					num_revealed++;
				}

				// left
				if (observations[8] == 10.0) {
					num_flagged++;
				} else if (observations[8] != -5.0) {
					num_revealed++;
				}

				if (num_flagged + num_revealed != 8) {
					if (num_mines == num_flagged) {
						actions.push_back(Action(MINESWEEPER_ACTION_DOUBLECLICK));

						iter_made_progress = true;
					} else if (num_revealed + num_mines == 8) {
						// top left
						if (observations[1] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
						}

						// top
						if (observations[2] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
						}

						// top right
						if (observations[3] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
						}

						// right
						if (observations[4] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
						}

						// bottom right
						if (observations[5] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
						}

						// bottom
						if (observations[6] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
						}

						// bottom left
						if (observations[7] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
						}

						// left
						if (observations[8] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
						}

						iter_made_progress = true;
					}
				}
			}

			if (observations[8] == -10.0) {
				if (observations[2] == -10.0) {
					if (iter_made_progress) {
						iter_made_progress = false;

						this->state = MINESWEEPER_SOLUTION_STATE_TO_BOTTOM;

						step_done = false;
					} else {
						done = true;

						step_done = true;
					}
				} else {
					done = false;
					actions.push_back(Action(MINESWEEPER_ACTION_UP));

					step_done = true;

					this->state = MINESWEEPER_SOLUTION_STATE_TRAVERSE_RIGHT;
				}
			} else {
				done = false;
				actions.push_back(Action(MINESWEEPER_ACTION_LEFT));

				step_done = true;
			}

			break;
		case MINESWEEPER_SOLUTION_STATE_TRAVERSE_RIGHT:
			if (observations[0] > 0.0 && observations[0] < 10.0) {
				int num_mines = (int)observations[0];

				int num_flagged = 0;
				int num_revealed = 0;

				// top left
				if (observations[1] == 10.0) {
					num_flagged++;
				} else if (observations[1] != -5.0) {
					num_revealed++;
				}

				// top
				if (observations[2] == 10.0) {
					num_flagged++;
				} else if (observations[2] != -5.0) {
					num_revealed++;
				}

				// top right
				if (observations[3] == 10.0) {
					num_flagged++;
				} else if (observations[3] != -5.0) {
					num_revealed++;
				}

				// right
				if (observations[4] == 10.0) {
					num_flagged++;
				} else if (observations[4] != -5.0) {
					num_revealed++;
				}

				// bottom right
				if (observations[5] == 10.0) {
					num_flagged++;
				} else if (observations[5] != -5.0) {
					num_revealed++;
				}

				// bottom
				if (observations[6] == 10.0) {
					num_flagged++;
				} else if (observations[6] != -5.0) {
					num_revealed++;
				}

				// bottom left
				if (observations[7] == 10.0) {
					num_flagged++;
				} else if (observations[7] != -5.0) {
					num_revealed++;
				}

				// left
				if (observations[8] == 10.0) {
					num_flagged++;
				} else if (observations[8] != -5.0) {
					num_revealed++;
				}

				if (num_flagged + num_revealed != 8) {
					if (num_mines == num_flagged) {
						actions.push_back(Action(MINESWEEPER_ACTION_DOUBLECLICK));

						iter_made_progress = true;
					} else if (num_revealed + num_mines == 8) {
						// top left
						if (observations[1] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
						}

						// top
						if (observations[2] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
						}

						// top right
						if (observations[3] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
						}

						// right
						if (observations[4] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
						}

						// bottom right
						if (observations[5] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
						}

						// bottom
						if (observations[6] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
						}

						// bottom left
						if (observations[7] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_DOWN));
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
							actions.push_back(Action(MINESWEEPER_ACTION_UP));
						}

						// left
						if (observations[8] == -5.0) {
							actions.push_back(Action(MINESWEEPER_ACTION_LEFT));
							actions.push_back(Action(MINESWEEPER_ACTION_FLAG));
							actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));
						}

						iter_made_progress = true;
					}
				}
			}

			if (observations[4] == -10.0) {
				if (observations[2] == -10.0) {
					if (iter_made_progress) {
						iter_made_progress = false;

						this->state = MINESWEEPER_SOLUTION_STATE_TO_BOTTOM;

						step_done = false;
					} else {
						done = true;

						step_done = true;
					}
				} else {
					done = false;
					actions.push_back(Action(MINESWEEPER_ACTION_UP));

					step_done = true;

					this->state = MINESWEEPER_SOLUTION_STATE_TRAVERSE_LEFT;
				}
			} else {
				done = false;
				actions.push_back(Action(MINESWEEPER_ACTION_RIGHT));

				step_done = true;
			}

			break;
		}

		if (step_done) {
			break;
		}
	}
}
