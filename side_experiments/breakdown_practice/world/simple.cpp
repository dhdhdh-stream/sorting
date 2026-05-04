#include "simple.h"

using namespace std;

const int SPOT_TYPE_EMPTY = 0;
const int SPOT_TYPE_BLOCKED = 1;

const vector<vector<int>> MAP{
	vector<int>{SPOT_TYPE_BLOCKED, SPOT_TYPE_BLOCKED, SPOT_TYPE_BLOCKED, SPOT_TYPE_EMPTY, SPOT_TYPE_BLOCKED},
	vector<int>{SPOT_TYPE_BLOCKED, SPOT_TYPE_BLOCKED, SPOT_TYPE_BLOCKED, SPOT_TYPE_EMPTY, SPOT_TYPE_BLOCKED},
	vector<int>{SPOT_TYPE_BLOCKED, SPOT_TYPE_BLOCKED, SPOT_TYPE_BLOCKED, SPOT_TYPE_EMPTY, SPOT_TYPE_BLOCKED},
	vector<int>{SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY},
	vector<int>{SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY, SPOT_TYPE_EMPTY},
};

Simple::Simple() {
	this->current_x = 2;
	this->current_y = 3;

	this->hit_target = false;
	this->is_fail = false;
}

void Simple::perform_action(int action) {
	switch (action) {
	case ACTION_UP:
		if (this->current_y < (int)MAP.size()-1) {
			if (MAP[this->current_y+1][this->current_x] != SPOT_TYPE_BLOCKED) {
				this->current_y++;
			}
		}
		break;
	case ACTION_RIGHT:
		if (this->current_x < (int)MAP[this->current_y].size()-1) {
			if (MAP[this->current_y][this->current_x+1] != SPOT_TYPE_BLOCKED) {
				this->current_x++;
			}
		}
		break;
	case ACTION_DOWN:
		if (this->current_y > 0) {
			if (MAP[this->current_y-1][this->current_x] != SPOT_TYPE_BLOCKED) {
				this->current_y--;
			}
		}
		break;
	case ACTION_LEFT:
		if (this->current_x > 0) {
			if (MAP[this->current_y][this->current_x-1] != SPOT_TYPE_BLOCKED) {
				this->current_x--;
			}
		}
		break;
	case ACTION_CLICK:
		if (this->current_x == 3
				&& this->current_y == 0) {
			this->hit_target = true;
		} else {
			this->is_fail = true;
		}
		break;
	}
}

double Simple::score_result() {
	if (this->is_fail) {
		return -1.0;
	} else if (this->hit_target) {
		return 1.0;
	} else {
		return 0.0;
	}
}

Problem* TypeSimple::get_problem() {
	return new Simple();
}

int TypeSimple::num_possible_actions() {
	return 5;
}
