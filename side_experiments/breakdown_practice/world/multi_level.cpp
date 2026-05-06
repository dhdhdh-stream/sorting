#include "multi_level.h"

using namespace std;

const int SPOT_TYPE_EMPTY = 0;
const int SPOT_TYPE_BLOCKED = 1;

const vector<vector<int>> MAP{
	vector<int>{SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY},
	vector<int>{SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY},
	vector<int>{SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY},
	vector<int>{SPOT_TYPE_BLOCKED, SPOT_TYPE_EMPTY, SPOT_TYPE_BLOCKED},
	vector<int>{SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY},
	vector<int>{SPOT_TYPE_BLOCKED, SPOT_TYPE_EMPTY, SPOT_TYPE_BLOCKED},
	vector<int>{SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY},
	vector<int>{SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY},
	vector<int>{SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY},
};

MultiLevel::MultiLevel() {
	this->current_x = 1;
	this->current_y = 4;

	this->hit_top = false;
	this->hit_bottom = false;
	this->is_fail = false;
}

void MultiLevel::perform_action(int action) {
	switch (action) {
	case MULTI_LEVEL_ACTION_UP:
		if (this->current_y < (int)MAP.size()-1) {
			if (MAP[this->current_y+1][this->current_x] != SPOT_TYPE_BLOCKED) {
				this->current_y++;
			}
		}
		break;
	case MULTI_LEVEL_ACTION_RIGHT:
		if (this->current_x < (int)MAP[this->current_y].size()-1) {
			if (MAP[this->current_y][this->current_x+1] != SPOT_TYPE_BLOCKED) {
				this->current_x++;
			}
		}
		break;
	case MULTI_LEVEL_ACTION_DOWN:
		if (this->current_y > 0) {
			if (MAP[this->current_y-1][this->current_x] != SPOT_TYPE_BLOCKED) {
				this->current_y--;
			}
		}
		break;
	case MULTI_LEVEL_ACTION_LEFT:
		if (this->current_x > 0) {
			if (MAP[this->current_y][this->current_x-1] != SPOT_TYPE_BLOCKED) {
				this->current_x--;
			}
		}
		break;
	case MULTI_LEVEL_ACTION_CLICK:
		if (this->current_x == 0
				&& this->current_y == 2) {
			this->hit_top = true;
		} else if (this->current_x == 2
				&& this->current_y == 7) {
			this->hit_bottom = true;
		} else {
			this->is_fail = true;
		}
		break;
	}
}

double MultiLevel::score_result() {
	if (this->is_fail) {
		return -1.0;
	} else if (this->current_x != 1
			|| this->current_y != 4) {
		return -1.0;
	} else {
		double score = 0.0;
		if (this->hit_top) {
			score += 0.5;
		}
		if (this->hit_bottom) {
			score += 0.5;
		}
		return score;
	}
}

Problem* TypeMultiLevel::get_problem() {
	return new MultiLevel();
};

int TypeMultiLevel::num_possible_actions() {
	return 5;
}
