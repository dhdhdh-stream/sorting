#ifndef ACTION_H
#define ACTION_H

#include <string>

const int LEFT = 0;
const int STAY = 1;
const int RIGHT = 2;

class Action {
public:
	double write;
	int move;

	Action(double write, int move) {
		this->write = write;
		this->move = move;
	}
	~Action() {
		// do nothing
	}

	std::string to_string() {
		std::string move_s;
		if (this->move == LEFT) {
			move_s = "LEFT";
		} else if (this->move == STAY) {
			move_s = "STAY";
		} else {
			move_s = "RIGHT";
		}
		
		return "(" + std::to_string(this->write) + "," + move_s + ")";
	}
};

const Action HALT(0.0, -1);
const Action EMPTY_ACTION(0.0, -1);

#endif /* ACTION_H */