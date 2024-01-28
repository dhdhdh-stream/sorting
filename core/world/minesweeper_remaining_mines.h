#ifndef MINESWEEPER_REMAINING_MINES_H
#define MINESWEEPER_REMAINING_MINES_H

#include <vector>

#include "state_scenario.h"

class MinesweeperRemainingMines : public StateScenario {
public:
	MinesweeperRemainingMines();

	double get_target_state();
};

#endif /* MINESWEEPER_REMAINING_MINES_H */