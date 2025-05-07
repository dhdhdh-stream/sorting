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
const int MINESWEEPER_ACTION_DOUBLECLICK = 6;
/**
 * - for visual problems, stick to moving focus and looking one square at a time
 *   - don't, e.g., use convolutional NN as far too expensive
 */

class Minesweeper : public Problem {
public:
	std::vector<std::vector<int>> world;
	std::vector<std::vector<bool>> revealed;
	std::vector<std::vector<bool>> flagged;
	int current_x;
	int current_y;

	bool hit_mine;

	Minesweeper();

	std::vector<double> get_observations();
	void perform_action(Action action);
	double score_result();

	#if defined(MDEBUG) && MDEBUG
	Problem* copy_and_reset();
	Problem* copy_snapshot();
	#endif /* MDEBUG */

	void print();

	double get_observation_helper(int x, int y);
	void reveal_helper(int x, int y);
};

class TypeMinesweeper : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
	Action random_action();
};

#endif /* MINESWEEPER_H */