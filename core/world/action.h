#ifndef ACTION_H
#define ACTION_H

#include <string>

const int LEFT = 0;
const int STAY = 1;
const int RIGHT = 2;

const int COMPOUND = 3;
const int LOOP = 4;

class Action {
public:
	double write;
	int move;

	int compound_index;

	Action(double write, int move) {
		this->write = write;
		this->move = move;
		this->compound_index = -1;
	}
	Action(int compound_index) {
		this->write = 0.0;
		this->move = COMPOUND;
		this->compound_index = compound_index;
	}
	~Action() {
		// do nothing
	}

	std::string to_string() {
		if (this->move == COMPOUND) {
			return "C " + std::to_string(this->compound_index);
		} else {
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
	}
};

const Action HALT(0.0, -1);
const Action EMPTY_ACTION(0.0, -1);

#endif /* ACTION_H */