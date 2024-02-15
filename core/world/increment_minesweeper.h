#ifndef INCREMENT_MINESWEEPER_H
#define INCREMENT_MINESWEEPER_H

#include <vector>

#include "problem.h"

const int INCREMENT_MINESWEEPER_ACTION_UP = 0;
const int INCREMENT_MINESWEEPER_ACTION_RIGHT = 1;
const int INCREMENT_MINESWEEPER_ACTION_DOWN = 2;
const int INCREMENT_MINESWEEPER_ACTION_LEFT = 3;
const int INCREMENT_MINESWEEPER_ACTION_CLICK = 4;
const int INCREMENT_MINESWEEPER_ACTION_FLAG = 5;

class IncrementMinesweeper : public Problem {
public:
	std::vector<std::vector<int>> world;
	std::vector<std::vector<bool>> revealed;
	std::vector<std::vector<bool>> flagged;
	int current_x;
	int current_y;

	int num_correct;

	bool hit_mine;

	IncrementMinesweeper();

	int num_actions();
	Action random_action();

	double get_observation();
	void perform_action(Action action);
	double score_result();

	Problem* copy_and_reset();

	void print();

private:
	void reveal_helper(int x, int y);
};

#endif /* INCREMENT_MINESWEEPER_H */