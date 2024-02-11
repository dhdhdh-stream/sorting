/**
 * - curriculum:
 *   - 0:
 *     - fetch remaining squares
 *     - fetch remaining mines
 *   - 1:
 *     - if 0 remaining mines, click all
 *     - if num squares equals num mines, flag all
 *   - 2:
 *     - fetch number of mines in certain squares
 *     - fetch number of non-mines in certain squares
 *   - 3:
 *     - if 0 remaining mines outside squares, click all
 *     - if num mine equals num outside squares, flag all
 */

#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <vector>

#include "problem.h"

const int MINESWEEPER_ACTION_UP = 0;
const int MINESWEEPER_ACTION_RIGHT = 1;
const int MINESWEEPER_ACTION_DOWN = 2;
const int MINESWEEPER_ACTION_LEFT = 3;
const int MINESWEEPER_ACTION_CLICK = 4;
const int MINESWEEPER_ACTION_FLAG = 5;

class Minesweeper : public Problem {
public:
	int world_size;

	std::vector<std::vector<int>> world;
	std::vector<std::vector<bool>> revealed;
	std::vector<std::vector<bool>> flagged;
	int current_x;
	int current_y;

	bool ended;

	Minesweeper();

	int num_actions();
	Action random_action();

	double get_observation();
	void perform_action(Action action);
	double score_result(int num_process);

	Problem* copy_and_reset();

	void print();

private:
	void reveal_helper(int x, int y);
};

#endif /* MINESWEEPER_H */