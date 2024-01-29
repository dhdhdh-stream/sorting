#include "minesweeper_open.h"

#include "constants.h"
#include "globals.h"
#include "minesweeper.h"

using namespace std;

MinesweeperOpen::MinesweeperOpen() {
	Minesweeper* minesweeper = new Minesweeper();

	minesweeper->world_size = 0;

	minesweeper->world = vector<vector<int>>(3, vector<int>(3, 0));

	uniform_int_distribution<int> mine_distribution(0, 2);
	if (mine_distribution(generator) == 0) {
		minesweeper->world[1][1] = -1;
	} else {
		geometric_distribution<int> num_surrounding_distribution(0.3);
		int num_surrounding = num_surrounding_distribution(generator);
		if (num_surrounding > 8) {
			num_surrounding = 8;
		}
		minesweeper->world[1][1] = num_surrounding;
	}

	minesweeper->revealed = vector<vector<bool>>(3, vector<bool>(3, false));
	minesweeper->flagged = vector<vector<bool>>(3, vector<bool>(3, false));

	uniform_int_distribution<int> revealed_distribution(0, 1);
	if (revealed_distribution(generator) == 0) {
		if (minesweeper->world[1][1] == -1) {
			minesweeper->flagged[1][1] = true;
		} else {
			minesweeper->revealed[1][1] = true;
		}

		this->start_is_revealed = true;
	} else {
		this->start_is_revealed = false;
	}

	minesweeper->current_x = 1;
	minesweeper->current_y = 1;

	this->problem = minesweeper;
}

MinesweeperOpen::~MinesweeperOpen() {
	delete this->problem;
}

void MinesweeperOpen::get_attention(std::vector<int>& types,
									std::vector<Action>& actions,
									std::vector<std::string>& scopes) {
	// do nothing
}

void MinesweeperOpen::get_sequence(std::vector<int>& types,
								   std::vector<Action>& actions,
								   std::vector<std::string>& scopes) {
	types.push_back(STEP_TYPE_ACTION);
	actions.push_back(Action(MINESWEEPER_ACTION_CLICK));
	scopes.push_back("");
}

bool MinesweeperOpen::should_perform_sequence() {
	return !this->start_is_revealed;
}

string MinesweeperOpen::get_name() {
	return "minesweeper_open";
}
