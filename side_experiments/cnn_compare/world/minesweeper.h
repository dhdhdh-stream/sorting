#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <vector>

const int MINESWEEPER_WIDTH = 9;
const int MINESWEEPER_HEIGHT = 9;
const int MINESWEEPER_NUM_MINES = 10;
const int MINESWEEPER_STARTING_X = 4;
const int MINESWEEPER_STARTING_Y = 4;

class Minesweeper {
public:
	std::vector<std::vector<int>> world;
	std::vector<std::vector<bool>> revealed;
	std::vector<std::vector<bool>> flagged;
	int current_x;
	int current_y;

	Minesweeper();

	double get_observation_helper(int x, int y);

	double score_result();

	void print();

private:
	void reveal_helper(int x, int y);
};

#endif /* MINESWEEPER_H */